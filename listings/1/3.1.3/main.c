
#define _GNU_SOURCE			/* Для clone() */

#include <stdio.h>
#include <string.h>

#include <sys/types.h>		/* Для wait() */
#include <sys/wait.h>
#include <sched.h>			/* Для clone() */

char stack[1024];

/* Главная функция дочернего процесса */
int lwp(void *arg)
{
	strcpy((char *) arg, "Успешно вошли в дочерний процесс");

	return 0;
}

int main()
{
	char buf[100];

	/* Запускаем дочерний процесс */
	int child_pid = clone(
			& lwp,			/* Адрес главной функции дочернего процесса */
			stack + 1023,	/* Адрес вершины стека дочернего процесса */
			CLONE_VM | CLONE_SIGHAND | CLONE_FS | CLONE_FILES | SIGCHLD, /* Флаги */
			(void *) buf	/* Аргумент, передающийся в главную функцию дочернего процесса */
			);

	if(child_pid == -1)
		perror("Ошибка при запуске дочернего процесса");
	else
	{
		printf("\nРодительский процесс сообщает: PID дочернего = %d\n", child_pid);

		/* Ожидаем завершение дочернего процесса */
		wait(NULL);

		printf("Родительский процесс сообщает: %s\n\n", buf);
	}

	return 0;
}

