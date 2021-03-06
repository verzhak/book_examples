
#include <stdio.h>
#include <stdlib.h>		/* Для system() */

#include <unistd.h>		/* Для link() и unlink() */

/* Макрос для вывода номеров inode файлов и подкаталогов текущего каталога */
#define LS \
{\
	printf("\nНомера inode элементов каталога:\n\n");\
	system("ls -i");\
	printf("\n");\
}

int main()
{
	LS;

	printf("-----> Создание жесткой ссылки HL_main.c для файла main.c <-----\n");
	if(
		/* Создаем вторую жесткую ссылку на файл main.c (первая - сам main.c) -
		 * HL_main.c */
		link("main.c", "HL_main.c") == -1
	  )
		perror("Ошибка при создании жесткой ссылки");
	else
	{
		/* Жесткая ссылка создана успешно */

		LS;

		printf("-----> Удаление жесткой ссылки HL_main.c <-----\n\
(файл удален не будет, так как на него ссылается еще одна жесткая ссылка - main.c)\n");
		if(
			/* Удаляем HL_main.c - жесткую ссылку на файл main.c
			 *
			 * (у данного файла останется еще одна жесткая ссылка - main.c) */
			unlink("HL_main.c") == -1
		  )
			perror("Ошибка при удалении жесткой ссылки HL_main.c");
		else
			/* Удаление жесткой ссылки прошло успешно */
			LS;
	}

	return 0;
}

