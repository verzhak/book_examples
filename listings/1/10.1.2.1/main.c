
#include <stdio.h>

#include <stdlib.h>			/* Для exit() */
#include <unistd.h>			/* Для getpid() */
#include <signal.h>			/* Для kill(), signal() */
#include <sys/types.h>

/* Функция - обработчик сигналов SIGURG, SIGWINCH, SIGTERM */
void signal_proc(int signum)
{
	if(signum == SIGURG)
		fprintf(stderr, "\tПолучен сигнал SIGURG\n");
	else if(signum == SIGWINCH)
		fprintf(stderr, "\tПолучен сигнал SIGWINCH\n");
	else if(signum == SIGTERM) /* По получении SIGTERM мы не будем завершать процесс */
		fprintf(stderr, "\tПолучен сигнал SIGTERM\n");
}

int main()
{
	printf("\n");

	/* Устанавливаем действие по получении сигналов SIGURG, SIGWINCH, SIGTERM -
	 * обрабатывать функцией signal_proc() */
	if(
			signal(SIGURG, &signal_proc) == SIG_ERR
			||
			signal(SIGWINCH, &signal_proc) == SIG_ERR
			||
			signal(SIGTERM, &signal_proc) == SIG_ERR
	  )
		perror("Ошибка при установке обработчика сигнала");

	printf("Отправляем сигнал SIGURG\n");

	/* Отправляем SIGURG данному процессу */
	if(
			kill(getpid(), SIGURG) == -1
	  )
		perror("Ошибка при отправлении сигнала SIGURG");

	printf("Отправляем сигнал SIGWINCH\n");

	/* Отправляем SIGWINCH всем процессам из группы данного процесса */
	if(
			kill(0, SIGWINCH) == -1
	  )
		perror("Ошибка при отправлении сигнала SIGWINCH");

	printf("Отправляем сигнал SIGTERM\n");

	/* Отправляем SIGTERM данному процессу */
	if(
			kill(getpid(), SIGTERM) == -1
	  )
		perror("Ошибка при отправлении сигнала SIGTERM");

	printf("Отправляем сигнал SIGKILL\n\n");

	/* Отправляем SIGKILL данному процессу */
	if(
			kill(getpid(), SIGKILL) == -1
	  )
		perror("Ошибка при отправлении сигнала SIGKILL");

	return 0;
}

