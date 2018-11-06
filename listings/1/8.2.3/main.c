
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>		/* Для open() */
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>			/* Для close() */
#include <sys/mman.h>		/* Для mmap() и munmap() */

int main()
{
	printf("\nОткрытие файла /dev/zero\n\n");

	/* Открываем на чтение и запись файл /dev/zero */
	int fd  = open("/dev/zero", O_RDWR);

	if(
		fd == -1
	  )
		perror("Ошибка при открытии файла /dev/zero");
	else
	{
		/* Файл /dev/zero успешно открыт */

		const size_t ptr_size = 7 * 4096;

		printf("Создание неразделяемого отображения файла /dev/zero в память размером %d байт = %d килобайт = %d страниц\n\n",
				ptr_size, ptr_size / 1024, ptr_size / 4096);

		/* Предпишем ОС создать неразделяемое (MAP_PRIVATE) отображение файла /dev/zero в виртуальное адресное пространство процесса
		 *
		 * Размер отображения: 28 килобайт (ptr_size; 7 страниц по 4 килобайта)
		 * Защита отображения: (PROT_READ | PROT_WRITE) - чтение и запись */
		char *ptr = (char *) mmap(NULL, ptr_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

		if(
			ptr == MAP_FAILED
		  )
			perror("Ошибка при отображении файла /dev/zero в память");
		else
		{
			/* Отображение файла /dev/zero в память успешно создано */

			unsigned int x;

			printf("Заполнение созданного отображения случайными числами\n\n");

			/* Заполняем отображение случайными данными */
			for(x = 0; x < ptr_size; x++)
				ptr[x] = rand() % 256;

			printf("Удаление отображения в память файла /dev/zero\n\n");

			/* Удаляем отображение (ОС отметит как недействительные все страницы виртуального адресного пространства процесса,
			 * попадающие, хотя бы частично, в диапазон адресов [ptr, ptr + ptr_size)) */
			if(
				munmap(ptr, ptr_size) == -1
			  )
				perror("Ошибка при удалении отображения файла /dev/zero в память");
		}

		printf("Закрытие файла /dev/zero\n");

		/* Закрываем файл /dev/zero */
		close(fd);
	}

	printf("\n");

	return 0;
}

