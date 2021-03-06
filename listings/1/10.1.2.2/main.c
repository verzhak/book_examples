
#include <stdio.h>

#include <stdlib.h>		/* Для exit() */
#include <unistd.h>		/* Для fork(), getpid(), pause() */
#include <sys/types.h>	/* Для wait() */
#include <sys/wait.h>
#include <signal.h>		/* Для killpg() */

/* Глобальная переменная child содержит PID дочернего процесса в родительском процессе и
 * 0 в дочернем процессе */
int child;

/* Обработчик сигнала SIGSEGV в обоих процесса */
void SIGSEGV_proc(int notused)
{
	printf("Пришел сигнал SIGSEGV процессу %d\n", getpid());

	/* Если сигнал SIGSEGV обрабатывается в дочернем процессе,
	 * то процесс завершается */
	if(! child)
		exit(0);
}

int main()
{
	printf("\n");

	/* Создаем новую группу для процесса. Данный процесс становится ее лидером */
	if(
			setpgid(0, 0) == -1
	  )
	{
		perror("Ошибка при создании новой группы процесса");

		return -1;
	}

	/* На сигнал SIGSEGV устанавливаем обработчик -
	 * привязка будет унаследована дочерним процессом */
	if(
			signal(SIGSEGV, & SIGSEGV_proc) == SIG_ERR
	  )
	{
		perror("Ошибка при установке обработчика для сигнала SIGSEGV");

		return -1;
	}

	/* Создаем дочерний процесс */
	if(
		(child = fork()) == -1
	  )
		perror("Ошибка при создании дочернего процесса");
	else if (child)
	{
		/* Мы в родительском процессе */

		/* Выведем PID'ы обоих процессов */
		printf("PID родительского процесса = %d\nPID дочернего процесса = %d\nОба процесса - в одной группе процессов\n\n",
			getpid(), child);

		/* Отправление сигнала SIGSEGV процессам из группы данного процесса */
		killpg(0, SIGSEGV);

		/* Ожидаем завершения дочернего процесса */
		wait(NULL);

		printf("\n");
	}
	else
		/* Мы в дочернем процессе
		 * (дочерний процесс унаследует привязки функций - обработчиков к сигналам */
		pause();

	return 0;
}

