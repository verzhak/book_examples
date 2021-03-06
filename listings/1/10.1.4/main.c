
#include <stdio.h>

#include <unistd.h>		/* Для pause() */
#include <sys/wait.h>	/* Для wait() */
#include <sys/types.h>	/* Для kill() */
#include <signal.h>

/* Функция - обработчик сигнала SIGURG для дочернего процесса */
void SIGURG_child_proc(int notused)
{
	fprintf(stderr, "Дочерним процессом принят сигнал SIGURG\n");
}

int main()
{
	printf("\n");

	/* Запускаем дочерний процесс */
	int child = fork();

	if(child == -1)
	{
		perror("Ошибка при создании дочернего процесса");
		return -1;
	}
	else if(child)
	{
		/* Мы в родительском процессе */

		/* Родительский процесс блокируется на секунду */
		sleep(1);

		printf("Родительский процесс отправляет дочернему сигнал SIGWINCH\n");

		/* Родительский процесс отправляет дочернему сигнал SIGWINCH.
		 * Данный сигнал, по умолчанию, игнорируется и не должен возобновить дочерний процесс */
		if(
				kill(child, SIGWINCH) == -1
		  )
			perror("Ошибка при отправлении сигнала SIGWINCH");

		printf("Родительский процесс отправляет дочернему сигнал SIGURG\n");

		/* Родительский процесс отправляет дочернему сигнал SIGURG.
		 * Данный сигнал будет обрабатываться дочерним процессом => должен возобновить его выполнение */
		if(
				kill(child, SIGURG) == -1
		  )
			perror("Ошибка при отправлении сигнала SIGURG");

		/* Ждем, когда дочерний процесс завершится */
		wait(NULL);

		printf("\n");
	}
	else
	{
		/* Мы в дочернем процессе */

		/* Установим обработчик сигнала SIGURG */
		if(
				signal(SIGURG, &SIGURG_child_proc) == SIG_ERR
		  )
			perror("Ошибка при установке обработчика сигнала SIGURG");

		printf("Дочерний процесс начинает ожидать сигнал\n");

		/* Дочерний процесс ожидает любой неблокированный сигнал */
		pause();

		printf("Дочерний процесс возобновил выполнение\n");
	}

	return 0;
}

