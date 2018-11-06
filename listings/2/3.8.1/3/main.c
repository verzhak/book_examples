
#define _GNU_SOURCE		/* Для F_NOTIFY */

#include <stdio.h>
#include <stdlib.h>

#include <wait.h>		/* Для wait() */
#include <signal.h>		/* Для sigaction() */
#include <sys/types.h>	/* Для open() */
#include <sys/stat.h>
#include <fcntl.h>		/* Для fcntl() */
#include <unistd.h>

/* Обработчик сигнала SIGIO для родительского процесса */
void SIGIO_proc(int signum)
{
	printf("\tРодительский процесс получил сигнал SIGIO - уведомление о переименовании файла\n\n");

	printf("##########################\nСодержимое каталога после переименования:\n\n");
	system("ls");
	printf("##########################\n\n");
}

int main()
{
	printf("\n");

	char fname[] = "test_file";
	char new_fname[] = "newname_test_file";

	/* Запуск дочернего процесса */
	int child = fork();

	if(child == -1)
		perror("Ошибка при запуске дочернего процесса");
	else if (child)
	{
		/* Мы в родительском процессе */

		/* Установка для родительского процесса обработчика сигнала SIGIO */
		if(
				signal(SIGIO, & SIGIO_proc) == SIG_ERR
		  )
		{
			perror("Ошибка при установке обработчика сигнала SIGIO");

			kill(child, SIGKILL);
			return -1;
		}

		/* Создаем временный файл и открываем его на запись
		 *
		 * Владелец и группа пользователей файла - действительные владелец и группа пользователей процесса
		 * Права доступа к файлу - владелец может читать данные из файла и записывать данные в файл,
		 *                         прочие пользователи не обладают никакими правами на доступ к файлу */
		int fd = creat(fname, S_IRUSR | S_IWUSR);
		
		if(
				fd == -1
		  )
		{
			perror("Ошибка при создании файла");

			kill(child, SIGKILL);
			return -1;
		}

		/* Закрытие созданного файла */
		close(fd);

		printf("##########################\nСодержимое каталога до переименования:\n\n");
		system("ls");
		printf("##########################\n\n");

		/* Открытие на чтение текущего каталога */
		if(
				(fd = open(".", O_RDONLY)) == -1
		  )
		{
			perror("Ошибка при открытии каталога");

			kill(child, SIGKILL);
			return -1;
		}

		/* Отслеживать переименования файлов внутри каталога */
		if(
				fcntl(fd, F_NOTIFY, DN_RENAME) == -1
		  )
		{
			perror("Ошибка при вызове fcntl()");

			kill(child, SIGKILL);
			close(fd);
			return -1;
		}

		sleep(1);

		/* Возобновление дочернего процесса */
		if(
				kill(child, SIGCONT) == -1
		  )
		{
			perror("Ошибка при возобновлении дочернего процесса");

			kill(child, SIGKILL);
			close(fd);
			return -1;
		}

		/* Ожидание завершения дочернего процесса */
		wait(NULL);

		/* Закрытие дескриптора каталога */
		close(fd);

		/* Удаление файла (неизвестно, успешно ли прошла операция переименования =>
		 * неизвестно, какой из нижеследующих вызовов будет успешным) */
		unlink(fname);
		unlink(new_fname);

		printf("\n");
	}
	else
	{
		/* Мы в дочернем процессе */

		/* Дочерний процесс засыпает */
		kill(getpid(), SIGSTOP);

		printf("Переименование файла (в дочернем процессе): %s -> %s\n", fname, new_fname);

		/* Дочерний процесс переименовывает файл */
		if(
				rename(fname, new_fname) == -1
		  )
			perror("Ошибка при переименовании");
	}

	return 0;
}

