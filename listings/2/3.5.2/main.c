
/* Получение информации о логическом разделе или об образе логического раздела, отформатированным в одну из следующих
 * файловых систем: FAT 12/16/32 - программа читает и анализирует блок параметров BIOS раздела (BPB)
 *
 * Запуск программы:
 *		
 *		Для получения справки об использовании программы:
 *			
 *			./program.out -h
 *			./program.out --help 
 *
 *		Для анализа образа логического раздела:
 *			
 *			./program.out PATH
 *
 *				PATH - путь и имя образа. Процесс должен обладать достаточными правами для доступа к данному файлу
 *
 *		Для анализа логического раздела:
 *
 *			./program.out /dev/NAME
 *
 *				NAME - имя файла логического раздела. Процесс должен иметь действительным владельцем суперпользователя
 *				или пользователя, состоящего в группе пользователей "disk" (в OpenSUSE 11.1; вообще - в группе пользователей
 *				владельца данного файла)
 */

#define _XOPEN_SOURCE 500	/* Для pread() */

#include <stdio.h>
#include <string.h>
#include <stdint.h>			/* Для uint8_t, uint16_t и uint32_t */

#include <sys/types.h>		/* Для open()*/
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>			/* Для pread(), close() */

int main(const int argc, const char *argv[])
{
	printf("\n");

	/* Анализируем переданные в программу параметры
	 *
	 * Параметр должен быть один - путь и имя целевого файла или "-h / --help" */
	if(argc != 2 || ! (strcmp(argv[1], "-h") && strcmp(argv[1], "--help")))
	{
		fprintf(stderr, "/program.out\t[-h] [--help] [PATH]\n\nГде:\n\t-h или --help\t-\tвывести справочную информацию\n\tPATH\t\t-\tпуть и имя образа логического диска или файла логического диска\n\n");

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

	uint8_t buf[0x1FF];		/* Буфер размеров в 512 байт */
	
	if(
			/* Читаем два последних байта из первого сектора логического раздела -
			 * ими должны быть 0xAA55 */
			pread(fd, buf, 2, 0x1FE) == 2
		&&
			buf[0] == 0x55 && buf[1] == 0xAA
		&&
			/* Читаем первые три байта из первого сектора логического раздела -
			 * ими должны быть 0xEBxx90 */
			pread(fd, buf, 3, 0) == 3
		&&
			buf[0] == 0xEB && buf[2] == 0x90
	  )
	{
		/* Читаем информацию об ОС и расширенный блок параметров BIOS (BPB) -
		 * (8 + 59) байт по смещению 0x3 от начала первого сектора раздела */
		if(
				pread(fd, buf, 59, 3) == 59
		  )
		{
			/* Анализируем полученную информацию */
			printf("Информация о логическом разделе %s:\n", argv[1]);

			printf("\tИмя и версия ОС: %s\n", buf);
			printf("\tКоличество байт в секторе: %u байт\n", *((uint16_t *) (buf + 0x0B - 3)));
			printf("\tКоличество секторов на кластер: %u\n", buf[0x0D - 3]);
			printf("\tКоличество резервных секторов: %u\n", *((uint16_t *) (buf + 0x0E - 3)));
			printf("\tЧисло таблиц FAT: %u\n", buf[0x10 - 3]);
			printf("\tМаксимальное число элементов в корневом каталоге: %u\n", *((uint16_t *) (buf + 0x11 - 3)));

			printf("\tОбщее количество секторов на логическом диске: %u\n", *((uint16_t *) (buf + 0x13 - 3)));

			/* Первый способ определения размера логического диска:
			 * (общее количество секторов на логическом диске * число байт в секторе) / (1024 * 1024) */
			printf("\t\tОбъем логического диска (первый способ расчета): %5f мегабайт\n",
					(*((uint16_t *) (buf + 0x13 - 3))) * (*((uint16_t *) (buf + 0x0B - 3))) / (1024.0 * 1024.0));
			/* Второй способ определения размера логического диска (для FAT 16):
			 * 
			 * (
			 *		(
			 *			(количество байт в секторе * количество секторов в FAT) / 2
			 *		)
			 *		*
			 *		(количество секторов на кластер) * (количество байт в секторе)
			 *	)
			 *	/
			 *	(1024 * 1024)
			 *
			 * Данный способ используется в случаях, если (общее количество секторов на логическом диске) установлено в 0
			 * (в случае некоторых тестовых флэш-дисков, отформатированных в FAT16)
			 */
			printf("\t\tОбъем логического диска (второй способ расчета; FAT 16): %5f мегабайт\n",
					((*((uint16_t *) (buf + 0x0B - 3)) * (*((uint16_t *) (buf + 0x16 - 3))) / 2) * buf[0x0D - 3])
					* (*((uint16_t *) (buf + 0x0B - 3))) / (1024.0 * 1024.0));
			/* Второй способ определения размера логического диска (для FAT 32) */
			printf("\t\tОбъем логического диска (второй способ расчета; FAT 32): %5f мегабайт\n",
					((*((uint16_t *) (buf + 0x0B - 3)) * (*((uint16_t *) (buf + 0x16 - 3))) / 4) * buf[0x0D - 3])
					* (*((uint16_t *) (buf + 0x0B - 3))) / (1024.0 * 1024.0));

			printf("\tТип носителя: 0x%X\n", buf[0x15 - 3]);
			printf("\tКоличество секторов в одной FAT: %u\n", *((uint16_t *) (buf + 0x16 - 3)));
			printf("\tКоличество секторов на дорожке: %u\n", *((uint16_t *) (buf + 0x18 - 3)));
			printf("\tКоличество головок: %u\n", *((uint16_t *) (buf + 0x1A - 3)));
			printf("\tКоличество скрытых секторов: %u\n", *((uint32_t *) (buf + 0x1C - 3)));

			buf[0x36 - 3] = '\0';
			printf("\tМетка диска: [%s]\n", (char *) (buf + 0x2B - 3));
		}
		else
			fprintf(stderr, "Невозможно прочитать расширенный блок параметров BIOS (BPB) из файла %s\n", argv[1]);
	}
	else
		fprintf(stderr, "Файл %s не является образом логического диска или файлом логического диска\n",
				argv[1]);
	
	/* Закрываем файл */
	close(fd);
	
	printf("\n");

	return 0;
}
