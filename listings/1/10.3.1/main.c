
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/poll.h>	/* Для poll() */
#include <sys/types.h>	/* Для waitpid() */
#include <sys/wait.h>
#include <unistd.h>		/* Для pipe() */

int main()
{
	printf("\n");
	
	int child_pid, ping_fd[2], pong_fd[2];

	/* Открываем два неименнованных канала
	 *
	 * ping_fd[] - канал для отправки родительским процессом дочернему ping-сообщений 
	 * pong_fd[] - канал для отправки дочерним процессом родительскому pong-сообщений
	 *			   в ответ на ping-сообщения */
	if(
			pipe(ping_fd) == -1
			||
			pipe(pong_fd) == -1
	  )
	{
		perror("Ошибка при создании каналов");
		return 0;
	}

#define PING "PING ["
#define PONG "PONG ["
#define PING_LEN 6
#define PING_NUM 5		/* Количество обменов ping-pong-сообщениями */

	short x;
	char buf[100];

	/* Запускаем дочерний процесс */
	if((child_pid = fork()) == -1)
		perror("Ошибка при запуске дочернего процесса");
	else if(child_pid)
	{
		/* Мы в родительском процессе */
	
		/* Выполним (необязательное) требование POSIX - закроем по одному дескриптору
		 * каждого канала */
		close(ping_fd[0]);
		close(pong_fd[1]);

		size_t next;

		for(x = 0; x < PING_NUM; x++)
		{
			strcpy(buf, PING);

			/* Формируем случайный ключ ping-сообщения */
			for(next = PING_LEN; next < PING_LEN + 5; next++)
				buf[next] = rand() % 26 + 'A';

			buf[next] = ']';
			buf[next + 1] = '\0';

			printf("Родительский процесс: отправлен\t%s\n", buf);

			/* Отправляем ping-сообщение */
			if (write(ping_fd[1], buf, strlen(buf) + 1) == -1)
				perror("Ошибка при записи в канал");

			/* Ожидать поступление данных в pong-канал будем с помощью poll() */
			struct pollfd ufd = {.fd = pong_fd[0], .events = POLLIN, .revents = 0};
			if(poll(& ufd, 1, -1) == -1)
				perror("Ошибка в вызове poll");
			else
			{
				/* Читаем пришедшее pong-сообщение */
				if(read(pong_fd[0], buf, 100) == -1)
					perror("Ошибка при чтении данных из канала");

				printf("Родительский процесс: получен\t%s\n\n", buf);
			}
		}

		/* Закрываем оставшиеся дескрипторы */
		close(ping_fd[1]);
		close(pong_fd[0]);

		printf("\n");
	}
	else
	{
		/* Мы в дочернем процессе */

		/* Выполним (необязательное) требование POSIX - закроем по одному дескриптору
		 * каждого канала */
		close(ping_fd[1]);
		close(pong_fd[0]);

		char pong_buf[100];

		for(x = 0; x < PING_NUM; x++)
		{
			/* Отслеживаем появление ping-сообщения с помощью poll() */
			struct pollfd ufd = {.fd = ping_fd[0], .events = POLLIN, .revents = 0};
			if(poll(& ufd, 1, -1) == -1)
				perror("Ошибка в вызове poll");
			else if (read(ping_fd[0], buf, 100) == -1)	/* Читаем пришедшее ping-сообщение */
				perror("Ошибка при чтении данных из канала");

			printf("\tДочерний процесс: получен\t%s\n", buf);

			strcpy(pong_buf, PONG);
			strcat(pong_buf, buf + PING_LEN);	/* В pong-сообщении отправляем ключ, пришедший в ping'е */

			printf("\tДочерний процесс: отправлен\t%s\n", pong_buf);

			/* Отправляем pong-сообщение */
			if(write(pong_fd[1], pong_buf, strlen(pong_buf) + 1) == -1)
				perror("Ошибка при записи данных в канал");
		}

		/* Закрываем оставшиеся дескрипторы */
		close(ping_fd[0]);
		close(pong_fd[1]);
	}

	return 0;
}

