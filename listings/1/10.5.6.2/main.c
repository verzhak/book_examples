
/* ######################################################################### 
 * Семафоры
 *
 * Родительский процесс: генерирует кванты ресурса
 * Дочерний процесс: потребляет 5 квантов ресурса за раз
 *
 * Ресурс: случайный числа, генерируемые функцией lrand48()
 *
 * Семафоры используются для распределения ресурса
 *
 * ######################################################################### */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>		/* Для time() */
#include <unistd.h>		/* Для pipe() */
#include <signal.h>		/* Для kill() */
#include <wait.h>		/* Для wait() */
#include <sys/types.h>	/* Для ftok(), semget(), semctl(), semop() */
#include <sys/ipc.h>
#include <sys/sem.h>

#define SALT 77777

int main()
{
	printf("\n");

	long data;
	int x, pfd[2];

	/* Открытие неименованного канала для обмена данными между
	 * родительским и дочерним процессами */
	if(
			pipe(pfd) == -1
		)
	{
		perror("Ошибка при создании канала");
		return -1;
	}

	/* Переменная, в которой будет сохранен ключ */
	key_t key;

	/* Объект, который будет использоваться при манипулировании значением
	 * семафора */
	struct sembuf sbuf;

	/* Запуск дочернего процесса */
	int id, child = fork();

	if(child == -1)
		perror("Ошибка при запуске дочернего процесса");
	else if(child)
	{
		/* Мы в родительском процессе */

		/* Инициализация датчика случайных чисел временем в секундах, прошедшем с
		 * момента начала Эпохи (01.01.1970) */
		srand48(time(NULL));

		/* Родительский процесс закрывает дескриптор канала, открытый для чтения */
		close(pfd[0]);

		/* На основе объекта файловой системы (каталог "/var") и соли (константа
		 * SALT) генерируется ключ семафора */
		if(
				(key = ftok("/var", SALT)) == -1
			)
		{
			perror("Ошибка при генерации ключа");
			
			kill(child, SIGTERM);
			return -1;
		}

		/* Создание семафора */
		if(
				(id = semget(key, 1, 0600 | IPC_CREAT)) == -1
			)
		{
			perror("Ошибка при создании семафора");
			
			kill(child, SIGTERM);
			return -1;
		}

		/* Заполнение полей объекта, отвечающего за операцию с семафором */

		sbuf.sem_num = 0;	/* Номер семафора */
		sbuf.sem_op = 1;	 /* Операция над семафором (увеличить значение семафора на 1) */
		sbuf.sem_flg = 0;	/* Флаги операции */

		for(x = 0; x < 15; x++)
		{
			/* Генерация очередного кванта ресурса */
			data = lrand48();
			
			printf("Родительский процесс пишет в канал: %ld\n", data);

			/* Сгенерированный квант ресурса становится доступным
			 * (случайное число помещается в канал) */
			write(pfd[1], & data, sizeof(data));

			/* Операция над семафором
			 *
			 * (смысл операции: родительский процесс сгенерировал очередной квант
			 * ресурса, сделал квант ресурса доступным (поместил в канал) и увеличил
			 * значение семафора, отвечающего за распределение ресурса, на 1) */
			if(semop(id, & sbuf, 1) == -1)
			{
				perror("Ошибка при изменении значения семафора");

				kill(child, SIGTERM);
				return -1;
			}
		}

		/* Засыпание в ожидании того, что дочерний процесс заблокируется
		 * вызовом semop() */
		sleep(1);

		/* Удаление семафора */
		if(
				semctl(id, 0, IPC_RMID) == -1
			)
			perror("Ошибка при удалении семафора");

		/* Закрытие открытого на запись дескриптора канала */
		close(pfd[1]);

		/* Ожидание завершения дочернего процесса */
		do
			wait(& x);
		while(! WIFEXITED(x));

		printf("\n");
	}
	else
	{
		/* Мы в дочернем процессе */

		/* Дочерний процесс закрывает дескриптор канала, открытый для записи */
		close(pfd[1]);

		/* На основе объекта файловой системы (каталог "/var") и соли (константа
		 * SALT) генерируется ключ семафора */
		if(
				(key = ftok("/var", SALT)) == -1
			)
		{
			perror("Ошибка при генерации ключа");
			
			kill(getppid(), SIGTERM);
			return -1;
		}

		/* Получение идентификатора семафора */
		if(
				(id = semget(key, 1, 0600 | IPC_CREAT)) == -1
			)
		{
			perror("Ошибка при получении идентификатора семафора");
			
			kill(getppid(), SIGTERM);
			return -1;
		}

		/* Заполнение полей объекта, отвечающего за операцию с семафором */

		sbuf.sem_num = 0;	/* Номер семафора */
		sbuf.sem_op = -5;	/* Операция над семафором (ожидать значения семафора >= 5 и уменьшение его на 5) */
		sbuf.sem_flg = 0;	/* Флаги операции */

		/* Выполнение операции над семафором
		 *
		 * (смысл операции: дочерний процесс становится в очередь за ресурсом,
		 * ждет, пока не станут доступными пять квантов ресурса, и использует все
		 * пять квантов ресурса) */
		while(
				semop(id, & sbuf, 1) != -1
			 )
		{
			printf("\tДочерний процесс принял очередные 5 чисел\n");

			/* Чтение из канала и вывод случайных чисел */
			for(x = 0; x < 5; x++)
				if(
						read(pfd[0], & data, sizeof(data)) == -1
					)
					perror("Ошибка при чтении данных из канала");
				else
					printf("\t\tДочерний процесс читает из канала: %ld\n", data);
		}

		/* Закрытие открытого на чтение дескриптора канала */
		close(pfd[0]);
	}

	return 0;
}

