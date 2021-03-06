
#include <stdio.h>

#include <sys/types.h>	/* Для fork() и vfork() */
#include <unistd.h>		/* Для execve(), execl() и execvp() */

int main()
{
	printf("\n");

	pid_t child_pid;

	/* Запускаем первый дочерний процесс */
	if(
			(child_pid = fork()) == -1
	  )
		perror("Ошибка при создании первого дочернего процесса");
	else if(child_pid)
	{
		/* Мы в родительском процессе. Запускаем второй дочерний процесс */
		if(
				(child_pid = vfork()) == -1
		  )
			perror("Ошибка при создании второго дочернего процесса");
		else if(child_pid)
		{
			/* Мы в родительском процессе */

			/* Подготавливаем список аргументов */
			char *argv[] = {"echo", "-e", "Родительский процесс: execvp\n", NULL};

			/* Загружаем исполняемый файл программы echo и передаем ему выполнение */
			execvp("echo", argv);

			perror("Ошибка при вызове execvp"); /* Данный вызов достижим, если execvp действительно завершился ошибкой */
		}
		else
		{
			/* Мы во втором дочернем */

			/* Загружаем исполняемый файл /bin/echo и передаем ему выполнение */
			execl("/bin/echo", "/bin/echo", "Дочерний процесс: vfork + execl", NULL);

			_exit(0); /* Если execl завершилось неудачей - обязательно сделаем системный вызов exit */
		}
	}
	else
	{
		/* Мы в первом дочернем процессе */

		/* Подготавливаем список аргументов */
		char *argv[] = {"/bin/echo", "Дочерний процесс: fork + execve", NULL};

		/* Загружаем исполняемый файл /bin/echo и передаем ему выполнение */
		execve("/bin/echo", argv, NULL);
		
		perror("Ошибка при вызове execve"); /* Данный вызов достижим, если execve действительно завершился ошибкой */
	}

	return 0; /* Достижима в случае ошибки в вызовах execve() или execvp() */
}

