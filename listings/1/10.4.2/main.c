
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include <wait.h>		/* Для wait() */
#include <sys/types.h>	/* Для fork() */
#include <unistd.h>
#include <sys/mman.h>	/* Для mmap(), munmap() */

int main()
{
	int x;

#define MY_MAP_SIZE 1578	/* Произвольный размер отображения */

	/* Создаем разделяемое анонимное отображение */
	char* addr = (char *) mmap(NULL, MY_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	if(addr == MAP_FAILED)
	{
		perror("Ошибка при создании разделяемого анонимного отображения");
		return -1;
	}

	/* Запускаем дочерний процесс */ 
	int child = fork();

	if(child == -1)
		perror("Ошибка при создании дочернего процесса");
	else if(child)
	{
		/* Мы в родительском процессе */

		/* В байты [1; 2047] файла пишем 0 либо 1 */
		for(x = 1; x < MY_MAP_SIZE; x++)
			addr[x] = rand() % 2;

		/* В нулевой байт отображения (который играет роль флага) пишем
		 * единицу - признак того, что в байт [1; 2047] записаны случайные символы */
		*addr = 1;
		
		/* Удаляем отображение */
		if(
				munmap((void *) addr, MY_MAP_SIZE) == -1
		  )
			perror("Ошибка при удалении отображения");

		/* Ожидаем завершение дочернего процесса */
		wait(NULL);
	}
	else
	{
		/* Мы в дочернем процессе */

		/* Ждем, пока первый байт не будет установлен в единицу */
		while(!(*addr));

		printf("\n-------------------------------[Начало сообщения]-------------------------------\n");

		/* Выводим на экран байты [1; 2047] */
		for(x = 0; x < MY_MAP_SIZE; x++)
			printf("%u", addr[x]);

		printf("\n-------------------------------[Конец  сообщения]-------------------------------\n\n");

		/* Удаляем отображение */
		if(
				munmap((void *) addr, MY_MAP_SIZE) == -1
		  )
			perror("Ошибка при удалении отображения");
	}

	return 0;
}

