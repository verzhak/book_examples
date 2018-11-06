
#include <stdio.h>

#include <sys/types.h>	/* Для opendir(), closedir() */
#include <dirent.h>		/* Для opendir(), readdir(), closedir() и структуры данных dirent */

int main()
{
	printf("\n");

	/* Открываем каталог /var/log
	 *
	 * (Указатель на дескриптор каталога сохраняется в ct) */
	DIR *ct = opendir("/var/log");

	if(ct == NULL)
	{
		perror("Ошибка при открытии каталога /var/log");

		return -1;
	}

	printf("/var/log :\n");

	struct dirent *next;

	while(
			/* Последовательно читаем информацию о каждом элементе каталога до
			 * тех пор, все элементы не будут просмотрены */
			(next = readdir(ct)) !=	NULL
		 )
	{
		/* Определяем inode найденного объекта файловой системы */
		printf("\tinode = %lu\t\t", next->d_ino);
		
		/* Определяем тип найденного элемента файловой системы */
		switch(next->d_type)
		{
			case DT_BLK:
				{
					printf("Блочное устройство\t");
					break;
				}
			case DT_CHR:
				{
					printf("Символьное устройство");
					break;
				}
			case DT_DIR:
				{
					printf("Каталог\t\t\t");
					break;
				}
			case DT_FIFO:
				{
					printf("Именованный канал\t");
					break;
				}
			case DT_LNK:
				{
					printf("Мягкая (символьная) ссылка");
					break;
				}
			case DT_REG:
				{
					printf("Обычный файл\t\t");
					break;
				}
			case DT_SOCK:
				{
					printf("Сокет\t\t\t");
					break;
				}
			default:
				printf("Тип описываемого объекта не определен");
		}

		/* Наконец, определяем имя найденного объекта файловой системы */
		printf("\t%s\n", next->d_name);
	}

	if(
		/* Закрываем каталог */
		closedir(ct) == -1
	  )
		perror("Ошибка при закрытии каталога /var/log");

	printf("\n");

	return 0;
}

