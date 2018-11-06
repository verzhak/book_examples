
#include <stdio.h>
#include <time.h>		/* Для ctime() */

#include <sys/types.h>	/* Для stat() */
#include <sys/stat.h>
#include <unistd.h>

int main()
{
	printf("\n");

	struct stat st;

	if(
			/* Получаем информацию о файле ./main.c (это данный файл - то есть файл
			 * с исходным кодом данной программы) */
			stat("./main.c", & st) == -1
	  )
		perror("Ошибка при получении информации о файле");
	else
	{
		/* Информация получена - выводим ее в стандартный поток вывода
		 *
		 * (модификаторы к printf() подобраны методом проб и ошибок, исходя из предположения,
		 * что все поля структуры данных stat базируются на целочисленных типах языка C) */

		printf("Файл ./main.c :\n\n");
		printf("\tНомер устройства: %llu = (%llu, %llu)\n\n", st.st_dev, (st.st_dev & 0xFFFFFF00) >> 8, st.st_dev & 0xFF);
		printf("\tНомер inode: %lu\n\n", st.st_ino);

		printf("\tТип объекта файловой системы:\n");
		switch(st.st_mode & S_IFMT)
		{
			case S_IFREG:
				{
					printf("\t\tОбычный файл\n");
					break;
				}
			case S_IFDIR:
				{
					printf("\t\tКаталог\n");
					break;
				}
			case S_IFCHR:
				{
					printf("\t\tСимвольное устройство\n");
					break;
				}
			case S_IFBLK:
				{
					printf("\t\tБлочное устройство\n");
					break;
				}
			case S_IFIFO:
				{
					printf("\t\tИменованный канал\n");
					break;
				}
			case S_IFLNK:
				{
					printf("\t\tМягкая (символьная) ссылка\n");
					break;
				}
			case S_IFSOCK:
				{
					printf("\t\tСокет\n");
					break;
				}
			default:
				printf("\t\tНеизвестный тип объекта файловой системы\n");
		}

		printf("\n\tБит set-user-id %s\n", (st.st_mode & S_ISUID) ? "установлен" : "не установлен");
		printf("\tБит set-group-id %s\n", (st.st_mode & S_ISGID) ? "установлен" : "не установлен");
		printf("\tБит \"sticky\" %s\n\n", (st.st_mode & S_ISVTX) ? "установлен" : "не установлен");

		printf("\tПрава доступа к объекту:\n");
		printf("\t\tВладелец = %s %s %s\n",
				(st.st_mode & S_IRUSR) ? "[ Чтение ]" : "",
				(st.st_mode & S_IWUSR) ? "[ Запись ]" : "",
				(st.st_mode & S_IXUSR) ? "[ Исполнение ]" : "");
		printf("\t\tГруппа пользователей владельца = %s %s %s\n",
				(st.st_mode & S_IRGRP) ? "[ Чтение ]" : "",
				(st.st_mode & S_IWGRP) ? "[ Запись ]" : "",
				(st.st_mode & S_IXGRP) ? "[ Исполнение ]" : "");
		printf("\t\tОстальные пользователи = %s %s %s\n\n",
				(st.st_mode & S_IROTH) ? "[ Чтение ]" : "",
				(st.st_mode & S_IWOTH) ? "[ Запись ]" : "",
				(st.st_mode & S_IXOTH) ? "[ Исполнение ]" : "");

		printf("\tКоличество жестких ссылок на объект: %d\n\n", st.st_nlink);

		printf("\tID владельца: %d\n", st.st_uid);
		printf("\tGID группы пользователей владельца: %d\n\n", st.st_gid);

		printf("\tРазмер файла: %lu байт == %.1f килобайт == %.5f мегабайт\n",
				st.st_size, st.st_size / 1024.0, st.st_size / (1024.0 * 1024.0));
		printf("\tРазмер блока в файловой системе: %lu байт\n", st.st_blksize);
		printf("\tФайл занимает %lu блоков размером по 512 байт (%lu байт == %lu блока(ов))\n\n",
				st.st_blocks, st.st_blocks * 512, st.st_blocks * 512 / st.st_blksize);

		printf("\tПоследнее обращение к файлу: %s", ctime(& st.st_atime));
		printf("\tПоследняя модификация файла: %s", ctime(& st.st_mtime));
		printf("\tСоздание файла: %s\n", ctime(& st.st_ctime));
	}

	return 0;
}

