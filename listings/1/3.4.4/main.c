
#include <stdio.h>
#include <unistd.h>			/* Для sleep() */
#include <stdlib.h>			/* Для exit() */

#include <sys/types.h>		/* Для wait(), waitpid(), wait4() */
#include <sys/wait.h>
#include <sys/time.h>		/* Для wait4() */
#include <sys/resource.h>

int main()
{
	int status;

	/* Запускаем первый дочерний процесс */
	pid_t ret_pid, child_pid = fork();

	if(child_pid == -1)
		perror("Ошибка при создании первого дочернего процесса");
	else if(! child_pid)
	{
		/* Мы в первом дочернем процессе */

		sleep(5);	/* Дочерний процесс засыпает на 5 секунд */

		exit(11);	/* Дочерний процесс завершается с кодом 11 */
	}

	/* Мы в родительском процессе */

	printf("\nPID первого дочернего процесса = %d\n", child_pid);

	/* Первый дочерний процесс отслеживаем с помощью wait() */
	if((ret_pid = wait(& status)) == -1)
		perror("Вызов wait() завершился ошибкой");
	else
		printf("Дочерний процесс с PID = %d завершился. Код возврата = %d\n", ret_pid, WEXITSTATUS(status));

	/* Запускаем второй дочерний процесс */
	if((child_pid = fork()) == -1)
		perror("Ошибка при создании второго дочернего процесса");
	else if(! child_pid)
	{
		/* Мы во втором дочернем процессе */

		sleep(5);	/* Дочерний процесс засыпает на 5 секунд */

		exit(55);	/* Дочерний процесс завершается с кодом 55 */
	}

	/* Мы в родительском процессе */

	printf("\nPID второго дочернего процесса = %d\n", child_pid);

	/* Второй дочерний процесс отслеживаем с помощью waitpid() */
	if((ret_pid = waitpid(child_pid, & status, 0)) == -1)
		perror("Вызов waitpid() завершился ошибкой");
	else
		printf("Дочерний процесс с PID = %d завершился. Код возврата = %d\n", ret_pid, WEXITSTATUS(status));

	/* Запускаем третий дочерний процесс */
	if((child_pid = fork()) == -1)
		perror("Ошибка при создании третьего дочернего процесса");
	else if(! child_pid)
	{
		/* Мы в третьем дочернем процессе */

		/* Займем процессор (бес)полезными действиями */
		int x;
		long long avg, total;

		for(x = 0; x < rand() % 100; x++)
		{
			total += rand();
			avg /= 11;
		}

		exit(77);	/* Дочерний процесс завершается с кодом 77 */
	}

	/* Мы в родительском процессе */

	printf("\nPID третьего дочернего процесса = %d\n", child_pid);

	/* Третий дочерний процесс отслеживаем с помощью wait4() */

	struct rusage child_rusage;
	
	if((ret_pid = wait4(child_pid, & status, 0, & child_rusage)) == -1)
		perror("Вызов waitpid() завершился ошибкой");
	else
		printf("Дочерний процесс с PID = %d завершился. Код возврата = %d. Затраченное процессом время = %ld миллисекунд\n\n",
				ret_pid, WEXITSTATUS(status), child_rusage.ru_stime.tv_usec);

	return 0;
}

