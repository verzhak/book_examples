
#include <stdio.h>
#include <stdlib.h>

#include <alloca.h>		/* Для alloca() */

int main()
{
	printf("\nВыделение динамической памяти на стеке с помощью функции alloca()\n");

#define BUF_SIZE 1024

	/* Предписываем ОС выделить процессу BUF_SIZE байт памяти на стеке */
	char *buf = (char *) alloca(BUF_SIZE);

	if(buf == NULL)
		perror("Ошибка при выделении динамической памяти на стеке с помощью функции allocа()");
	else
	{
		int x;

		printf("Динамическая память успешно выделена процессу\n");
		printf("Заполнение выделенной области памяти случайными данными\n");

		/* Заполняем выделенную область виртуального адресного пространства процесса случайными данными */
		for(x = 0; x < BUF_SIZE; x++)
			buf[x] = rand() % 256;

		printf("Заполнение выделенной области памяти случайными данными успешно завершено\n");
	}

	printf("\n");

	return 0;
}

