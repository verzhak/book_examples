
#include <stdio.h>
#include <stdlib.h>		/* Для system() */

#include <unistd.h>		/* Для symlink() и unlink() */

/* Макрос для вывода подробной информации о файлах и подкаталогах текущего каталога */
#define LS \
{\
	printf("\nСодержимое каталога:\n\n");\
	system("ls -l");\
	printf("\n");\
}

int main()
{
	LS;

	printf("-----> Создание мягкой (символьной) ссылки PROGRAM.OUT на файл program.out <-----\n");
	if(
		/* Создаем мягкую (символьную) ссылку PROGRAM.OUT на файл program.out */
		symlink("./program.out", "PROGRAM.OUT") == -1
	  )
		perror("Ошибка при создании жесткой ссылки");
	else
	{
		/* Мягкая ссылка создана успешно */

		LS;

		printf("-----> Удаление мягкой (символьной) ссылки PROGRAM.OUT <-----\n");
		if(
			/* Удаляем PROGRAM.OUT - мягкую (символьную) ссылку на program.out */
			unlink("PROGRAM.OUT") == -1
		  )
			perror("Ошибка при удалении жесткой ссылки HL_main.c");
		else
			/* Удаление мягкой ссылки прошло успешно */
			LS;
	}

	return 0;
}


