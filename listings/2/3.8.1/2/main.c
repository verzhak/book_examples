
#define _GNU_SOURCE		/* Для F_SETLEASE */

#include <stdio.h>

#include <signal.h>		/* Для sigaction() */
#include <wait.h>		/* Для wait() */
#include <sys/types.h>	/* Для open() */
#include <sys/stat.h>
#include <unistd.h>		/* Для fcntl() */
#include <fcntl.h>

/* Функция - обработчик сигнала SIGUSR1 для родительского процесса */
void SIGUSR1_proc(int signum, siginfo_t *si, void * notused)
{
	printf("\tПришел сигнал SIGUSR1 - к файлу обратились\n\tРодительский процесс снимает аренду с файла\n");

	/* Снятие аренды с файла
	 *
	 * (конкурирующий процесс сможет теперь открыть файл) */
	if(
			fcntl(si->si_fd, F_SETLEASE, F_UNLCK) == -1
	  )
		perror("Ошибка при снятии блокировки с файла");
}

int main()
{
	printf("\n");

	char fname[] = "./test_file";

	/* Запуск дочернего процесса */
	int child = fork();
	
	if(child == -1)
		perror("Ошибка при запуске дочернего процесса");
	else if (child)
	{
		/* Мы в родительском процессе */
		
		/* Объект act описывает устанавливаемое действие на сигнал SIGUSR1
		 *
		 * (флаг SA_SIGINFO предписывает передавать дополнительную информацию в обработчик
		 * (нас интересует поле si_fd структуры siginfo_t - в нем будет передан номер
		 * дескриптора файла)) */
		struct sigaction act = {.sa_sigaction = & SIGUSR1_proc, .sa_flags = SA_SIGINFO};

		/* Установка действия, описанного в act, на сигнал SIGUSR1 */
		if(
				sigaction(SIGUSR1, & act, NULL) == -1
		  )
		{
			perror("Ошибка при установке функции - обработчика сигнала SIGUSR1");

			kill(child, SIGKILL);
			return -1;
		}

		/* Создание / открытие файла на запись */
		int fd = open(fname, O_CREAT | O_WRONLY | O_ASYNC, S_IRUSR | S_IWUSR);

		if(fd == -1)
		{
			perror("Ошибка при создании файла");

			kill(child, SIGKILL);
			return -1;
		}

		if(
				/* Подмена сигнала SIGIO сигналом SIGUSR1 */
				fcntl(fd, F_SETSIG, SIGUSR1) == -1
				||
				/* Установка аренды на запись в файл */
				fcntl(fd, F_SETLEASE, F_WRLCK) == -1
		  )
		{
			perror("Ошибка при установке аренды на файл");

			close(fd);
			unlink(fname);
			kill(child, SIGKILL);
			return -1;
		}

		/* Гарантия приостановки дочернего процесса */
		sleep(5);

		if(
				/* Возобновление дочернего процесса */
				kill(child, SIGCONT) == -1
		  )
		{
			perror("Ошибка при возобновлении дочернего процесса");

			close(fd);
			unlink(fname);
			kill(child, SIGKILL);
			return -1;
		}
		
		sleep(1);

		/* Ожидание завершения дочернего процесса */
		wait(NULL);

		/* Закрытие файлового дескриптора */
		close(fd);

		/* Удаление файла */
		unlink(fname);

		printf("\n");
	}
	else
	{
		/* Мы в дочернем процессе */

		/* Приостановка дочернего процесса в ожидании создания файла и установки на него
		 * аренды родительским процессом */
		kill(getpid(), SIGSTOP);

		printf("Дочерний процесс пытается открыть файл\n");

		/* Попытка открыть файл на запись
		 *
		 * (сработают арендные ограничения, вызов будет блокирован, родительский процесс, получив SIGUSR1 (замену SIGIO),
		 * снимет аренду, что в итоге приведет к успешному возврату) */
		int fd = open(fname, O_WRONLY);

		if(
				fd == -1
		  )
			printf("Дочерний процесс не смог открыть файл\n");
		else
		{
			printf("Дочерний процесс открыл файл\n");

			/* Закрытие дескриптора файла */
			close(fd);
		}
	}

	return 0;
}

