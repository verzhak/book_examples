
#define _GNU_SOURCE			/* Для mremap() */

#include <stdio.h>

#include <sys/types.h>		/* Для open() */
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>			/* Для mremap() и close() */
#include <sys/mman.h>		/* Для mmap(), mremap() и munmap() */

int main()
{
	printf("\n");

	/* Создаем временный файл и открываем его на чтение и запись */
	int temp_fd = open("./test.file", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

	if(temp_fd == -1)
	{
		perror("Ошибка при создании временного файла");

		return -1;
	}

	/* Открываем на чтение пул энтропии (устройство /dev/urandom не блокирует чтение,
	 * даже если пул пуст, заменяя случайные данные псевдослучайными) */
	int rand_fd = open("/dev/urandom", O_RDONLY);

	if(rand_fd == -1)
	{
		perror("Ошибка при открытии на чтение пула случайных данных");

		close(temp_fd);

		return -1;
	}

	/* Заполняем временный файл мегабайтом (псевдо)случайных данных */
	long x;
	char temp[4096];
	for(x = 0; x < 1024 * 1024; x += 4096)
	{
		read(rand_fd, temp, 4096);

		write(temp_fd, temp, 4096);
	}
	
	/* Создаем отображение временного файла в память - процесс получает возможность читать (PROT_READ) данные из файла
	 * Отображение - неразделяемое (MAP_PRIVATE) */
	char *buf = mmap(NULL, 1024 * 1024, PROT_READ, MAP_PRIVATE, temp_fd, 0);
	
	/* Закрываем файловые дескрипторы (файловый дескриптор временного файла закрыт не будет,
	 * так как существует отображение данного файла в память - файловый дескриптор будет закрыт с удалением
	 * этого отображения) */
	close(temp_fd);
	close(rand_fd);

	if(buf == NULL)
	{
		perror("Ошибка при создании анонимного отображения");

		return -1;
	}

	/* Пытаемся прочитать 1000000-й байт из файла - попытка завершится успехом,
	 * так как отображение имеет размер равный одному мегабайту */
	printf("---> Попытка чтения 10000000-го байта <---\n");
	printf("\t%u\n\n", buf[1000000]);

	printf("---> Изменение размера отображения с (1024 * 1024) байт на 4096 байт <---\n\n");

	/* Изменяем размер отображения (с возможным перемещением - MREMAP_MAYMOVE) */
	buf = mremap(buf, 1024 * 1024, 4096, MREMAP_MAYMOVE);

	if(buf == NULL)
	{
		perror("Ошибка при изменении размера анонимного отображения");

		return -1;
	}

	/* Пытаемся прочитать 1000000-й байт из файла - попытка завершится неудачей (ошибкой сегментирования и
	 * аварийным завершением программы), так как размер отображения меньше 1000000 и равен 4096-и байтам */
	printf("---> Попытка чтения 10000000-го байта (ожидается ошибка сегментирования, так как отображение слишком мало) <---\n");
	printf("%u\n", buf[1000000]);

	/* Строки программы, ниже данной, никогда не получат управления -
	 * программа аварийно завершится ранее из-за ошибки сегментирования */

	/* Удаляем отображение файла */
	if(
		munmap(buf, 1024 * 1024) == -1
	  )
		perror("Ошибка при удалении отображения");

	/* Удаляем временный файл */
	if(
		unlink("./test.file") == -1
	  )
		perror("Ошибка при удалении временного файла");

	printf("\n");

	return 0;
}

