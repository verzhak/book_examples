
#include <stdio.h>

#include <sys/types.h>	/* Для fork() */
#include <unistd.h>

int main()
{
	pid_t child_pid = fork(); /* Создаем дочерний процесс */

	printf("\n");

	if(child_pid == -1)
		/* fork завершился ошибкой */
		perror("Ошибка при создании дочернего процесса");
	else if(child_pid)
		/* Мы в родительском процессе */
		printf("Родительский процесс (pid дочернего = %d)\n", child_pid);
	else
		/* Мы в дочернем процессе */
		printf("Дочерний процесс\n");

	printf("\n");

	return 0;
}

