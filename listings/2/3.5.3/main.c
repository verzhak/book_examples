
/* Получение таблицы разделов жесткого диска и вывод информации о разделах в стандартный поток вывода
 *
 * Формат запуска программы:
 *
 *		./program.out [-h] [--help] [PATH]
 *
 *		-h или --help	-	для получения справочной информации
 *		PATH			-	путь и имя образа жесткого диска или файла жесткого диска
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>		/* Для uint8_t и uint32_t */

#include <sys/types.h>	/* Для open() */
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>		/* Для close() */
#include <sys/uio.h>    /* Для readv() */

/* Массив названий файловых систем */
const char* s_fs_type[] =
{
	"Пустой раздел", "FAT12", "FAT16 (MS DOS 3.X)", "Расширенный (Extended) раздел (MS DOS)",
	"FAT16 (>= MS DOS 4.0)", "NTFS", "FAT32 (W95)", "FAT32 (W95; LBA)", "FAT16 (W95; LBA)",
	"Расширенный (Extended) раздел (W95)", "Linux Swap", "Linux", "Расширенный (Extended) раздел (Linux)",
	"Linux plaintext", "Linux LVM", "FreeBSD", "OpenBSD", "NetBSD",
	"Неизвестный раздел"
};

/* Функция, ставящая в соответствие переданному коду файловой системы индекс строки с названием ФС
 * из массива s_fs_type */
uint8_t fs_type(const uint8_t num)
{
	switch(num)
	{
		case 0:
			return 0;
		case 1:
			return 1;
		case 4:
			return 2;
		case 5:
			return 3;
		case 6:
			return 4;
		case 7:
			return 5;
		case 0xB:
			return 6;
		case 0xC:
			return 7;
		case 0xE:
			return 8;
		case 0xF:
			return 9;
		case 0x82:
			return 10;
		case 0x83:
			return 11;
		case 0x85:
			return 12;
		case 0x86:
			return 5;
		case 0x87:
			return 5;
		case 0x88:
			return 13;
		case 0x8E:
			return 14;
		case 0xA5:
			return 15;
		case 0xA6:
			return 16;
		case 0xA9:
			return 17;
	}

	return 18;
};

int main(const int argc, const char *argv[])
{
	printf("\n");

	/* Анализируем переданные в программу параметры
	 *
	 * Параметр должен быть один - путь и имя целевого файла или "-h / --help" */
	if(argc != 2 || ! (strcmp(argv[1], "-h") && strcmp(argv[1], "--help")))
	{
		fprintf(stderr, "/program.out\t[-h] [--help] [PATH]\n\nГде:\n\t-h или --help\t-\tвывести справочную информацию\n\tPATH\t\t-\tпуть и имя образа жесткого диска или файла жесткого диска\n\n");

		return -1;
	}

	/* Открываем целевой файл на чтение
	 *
	 * (использовать флаг O_LARGEFILE нет необходимости - даже если файл логического раздела имеет размер больший,
	 * чем 2 гигабайта, чтение будет происходить только по смещениям, попадающим в нижние 2 гигабайта -
	 * то есть по 32-х разрядным знаковым смещениям) */
	int fd = open(argv[1], O_RDONLY);

	if(fd == -1)
	{
		perror("Ошибка при открытии файла");
		printf("\n");

		return -1;
	}

	uint8_t x;

	/* Четыре буфера - по одному на запись в таблице разделов */
	uint8_t buf[4][16];
	struct iovec iov[4];

	/* Инициализируем описатели буферов - устанавливаем указатели на буферы и размеры буферов */
	for(x = 0; x < 4; x++)
	{
		iov[x].iov_base = (void *) buf[x];
		iov[x].iov_len = 16;
	}

	if(
			/* Позиционируем указатель чтения на начало таблицы разделов */
			lseek(fd, 0x1BE, SEEK_SET) == (off_t) -1
			||
			/* Если позиционирование прошло успешно - читаем таблицу разделов в буферы, описанные
			 * элементами массива iov */
			readv(fd, iov, 4) != (16 * 4)
	  )
	{
		perror("Ошибка при поиске таблицы разделов диска");
		printf("\n");
	}
	else
		/* Таблица разделов успешно считана */
		for(x = 0; x < 4; x++)
		{
			if(buf[x][4])
			{
				/* Анализируемый раздел не пуст */

				/* Выводим номер раздела */
				printf("Раздел %u\n", x);
				
				/* Проверяем, загружаемый ли раздел мы анализируем или нет */
				if(buf[x][0] == 0x80)
					printf("\tЗагружаемый раздел\n");
				else
					printf("\tНезагружаемый раздел\n");

				/* Выводим название файловой системы */
				printf("\tТип файловой системы: %s\n", s_fs_type[fs_type(buf[x][4])]);

				/* Выводим количество секторов в разделе */
				printf("\tКоличество секторов в разделе: 0x%X\n", * ((uint32_t *) & buf[x][0xC]));

				/* Высчитываем и выводим размер раздела */
				printf("\tРазмер раздела: %.1f мегабайт == %.1f гигабайт\n\n",
					(* ((uint32_t *) & buf[x][0xC])) / (2.0 * 1024.0),
					(* ((uint32_t *) & buf[x][0xC])) / (2.0 * 1024.0 * 1024.0));
			}
			else
				/* Раздел пуст */
				printf("Раздел %u пуст\n\n", x);
		}

	/* Закрываем файл */
	close(fd);
	
	return 0;
}

