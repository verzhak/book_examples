
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <time.h>		/* Для time() */
#include <signal.h>		/* Для kill() */
#include <wait.h>		/* Для wait() */
#include <fcntl.h>		/* Для open() */
#include <sys/types.h>	/* Для mkfifo() */
#include <sys/stat.h>


int main()
{
	/* Имя создаваемого именованного канала */
#define PIPE_FNAME "test_mkfifo"

	/* Создание именованного канала с правами доступа 0600
	 * 
	 * (чтение и запись разрешены только владельцу, остальным запрещено все) */
	if(
			mkfifo(PIPE_FNAME, 0600) == -1
	  )
	{
		perror("Ошибка при создании канала");

		return -1;
	}

	printf("\n#############################\nСодержимое текущего каталога:\n\n");

	/* Вывести листинг текущего каталога, дабы убедится в том, что
	 * именованный канал создан */
	system("ls");

	printf("\n#############################\n\n");

	/* Инициализация датчика случайных чисел временем в секундах,
	 * прошедшим с начала эпохи (01.01.1970) */
	srand48(time(NULL));

	/* Запуск дочернего процесса */
	int x, data, fd, child = fork();

	if(child == -1)
		perror("Ошибка при запуске дочернего процесса");
	else if (child)
	{
		/* Мы в родительском процесса */

		/* Родительский процесс открывает канал на запись */
		if(
				(fd = open(PIPE_FNAME, O_WRONLY)) == -1
			)
		{
			perror("Ошибка при открытии канала");

			kill(child, SIGTERM);
			return -1;
		}
		else
		{
			for(x = 0; x < 5; x++)
			{
				/* Генерирование случайного числа */
				data = lrand48() % 12345;

				printf("Отправлено: %d\n", data);

				/* Запись сгенерированного числа в канал */
				write(fd, (void *) & data, sizeof(data));
			}

			/* Закрытие дескриптора канала */
			close(fd);
		}

		/* Ожидание завершения дочернего процесса */
		wait(NULL);

		/* Удаление канала */
		if(
				unlink(PIPE_FNAME) == -1
		  )
			perror("Ошибка при удалении канала");

		printf("\n#############################\nСодержимое текущего каталога:\n\n");

		/* Вывести листинг текущего каталога, дабы убедится в том, что
		 * именованный канал удален */
		system("ls");

		printf("\n#############################\n\n");
	}
	else
	{
		/* Мы в дочернем процессе */

		/* Открытие канала на чтение */
		if(
				(fd = open(PIPE_FNAME, O_RDONLY)) == -1
			)
			perror("Ошибка при открытии канала");
		else
		{
			for(x = 0; x < 5; x++)
			{
				/* Чтение из канала очередного числа
				 *
				 * (чтение блокируется до тех пор, пока очередные данные не появятся
				 * в канале => необходимости в специальном функционале для ожидания
				 * появления данных нет) */
				read(fd, (void *) & data, sizeof(data));

				printf("\tПринято: %d\n", data);
			}

			/* Закрытие дескриптора канала */
			close(fd);
		}

	}

	return 0;
}

