
/* Сортировка файла, содержащего 17 произвольных целых со знаком
 *
 * Алгоритм работы программы:
 *
 *		1. Создание временного файла
 *		2. Заполнение временного файла произвольными целыми в диапазоне от 0 до 100
 *		3. Вывод содержимого временного файла
 *		4. Отображение временного файла в память
 *		5. Сортировка содержимого временного файла как обычного массива целых
 *		6. Удаление отображения файла => сброс отсортированного содержимого файла во внешнюю память
 *		7. Вывод содержимого временного файла
 *		8. Удаление временного файла
 */

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>		/* Для creat() */
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>			/* Для close() */
#include <sys/mman.h>		/* Для mmap() и munmap() */

/* Константы компилятора определяют размер временного файла:
 *
 * INT_FILE - размер файла в sizeof(int)-байтовых блоках
 * SIZE_FILE - размер файла в байтах */
#define INT_FILE 17
#define SIZE_FILE (17 * sizeof(int))

/* Функция quick_sort() производит сортировку (по алгоритму быстрой сортировки) массива array
 * размеров в array_size элементов */
void quick_sort(int *array, const size_t array_size);

/* Функция create_file() создает временный файл и заполняет его случайным содержимым */
int create_file();

/* Функция print_file() выводит содержимое файла, интерпретируя данные как целые со знаком */
int print_file();

/* Главная функция программы */
int main()
{
	printf("\n");

	/* Создаем файл */
	if(create_file() == -1)
		return -1;

	/* Выводим содержимое файла до сортировки */
	printf("Содержимое файла до сортировки:\n\n");
	if(print_file() == -1)
		return -1;

	printf("\n");

	/* Открываем файл на чтение и запись */
	int fd = open("./test.file", O_RDWR);
	
	if(
		fd == -1
	  )
	{
		perror("Ошибка при открытии временного файла");

		return -1;
	}

	/* Создаем отображение файла в память. Из отображения можно будет читать (PROT_READ) данные и в отображение
	 * можно будет записывать (PROT_WRITE) данные. Данные, записанные в отображение, будут сбрасываться и в сам файл (MAP_SHARED) */
	int *array = (int *) mmap(NULL, SIZE_FILE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	/* Освобождаем запись о дескрипторе файла в таблице файловых дескрипторов процесса - файловый дескриптор закрыт
	 * не будет, поскольку все еще существует отображение, связанное с файлом */
	close(fd);

	if(array == NULL)
	{
		fprintf(stderr, "Ошибка при создании отображения файла\n");
		
		return -1;
	}
	else
		/* Сортируем массив array размером в INT_FILE элементов */
		quick_sort(array, INT_FILE);

	/* Удаляем отображение - дескриптор файла закрывается, содержимое отображения сбрасывается в файл */
	if(
		munmap(array, SIZE_FILE) == -1
	  )
		perror("Ошибка при удалении отображения");

	/* Выводим содержимое файла после сортировки */
	printf("Содержимое файла после сортировки:\n\n");
	if(print_file() == -1)
		return -1;

	/* Удаляем файл */
	if(
		unlink("./test.file") == -1
	  )
		perror("Ошибка при удалении временного файла");

	printf("\n");

	return 0;
}

/* Функция quick_sort() производит сортировку (по алгоритму быстрой сортировки) массива array
 * размером в array_size элементов */
void quick_sort(int *array, const size_t array_size)
{
	if(array_size > 1)
	{
		int temp, medium = array[(array_size - 1) / 2];
		int begin = 0, end = array_size - 1;

		while(begin < end)
		{
			while(begin <= end && array[begin] < medium)
				begin++;

			while(begin <= end && array[end] > medium)
				end--;

			if(begin < end)
			{
				temp = array[begin];
				array[begin] = array[end];
				array[end] = temp;
			}

			if(begin <= end)
			{
				begin++;
				end--;
			}
		}

		if(begin < (array_size - 1))
			quick_sort(array + begin, array_size - begin);

		if(end > 0)
			quick_sort(array, end + 1);
	}
}

/* Функция create_file() создает временный файл и заполняет его случайным содержимым */
int create_file()
{
	/* Создаем временный файл и открываем его на запись
	 *
	 * Владелец и группа пользователей файла - действительные владелец и группа пользователей процесса
	 * Права доступа к файлу - владелец может читать данные из файла и записывать данные в файл,
	 *                         прочие пользователи не обладают никакими правами на доступ к файлу */
	int x, y, fd = creat("./test.file", S_IRUSR | S_IWUSR);

	if(fd == -1)
	{
		perror("Ошибка при создании временного файла");

		return -1;
	}

	/* Генерируем INT_FILE случайных целых в диапазоне от 0 до 100 и записываем их в файл */
	for(x = 0; x < INT_FILE; x++)
	{
		y = rand() % 100;
		
		if(
			write(fd, & y, sizeof(int)) < sizeof(int)
		  )
			perror("Ошибка при записи данных в файл");
	}

	/* Закрываем файл */
	close(fd);

	return 0;
}

/* Функция print_file() выводит содержимое файла, интерпретируя данные как целые со знаком */
int print_file()
{
	/* Открываем файл на чтение */
	int fd = open("./test.file", O_RDONLY);

	if(
		fd == -1
	  )
	{
		perror("Ошибка при открытии файла на чтение");

		return -1;
	}

	int buf;
	ssize_t read_sz = read(fd, & buf, sizeof(int));

	/* Читаем по sizeof(int) байт из файла, каждый раз выводя считанное целое.
	 * Считывание происходит до тех пор, пока не будет достигнут конец файла */
	while(read_sz == sizeof(int))
	{
		printf("\t%d\n", buf);
		
		read_sz = read(fd, & buf, sizeof(int));
	}

	/* Закрываем файл */
	close(fd);

	return 0;
}

