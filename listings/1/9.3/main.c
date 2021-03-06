
/* Алгоритм работы программы:
 *
 *		1. Явная привязка всех процессов программы к строго определенному процессору системы с помощью функции sched_setaffinity() -
 *		   необходимо для многопроцессорных и многоядерных систем для предотвращения распределения создаваемой процессами программы нагрузки
 *		2. Запуск UL_NUM бесполезных процессов, создающих нагрузку на процессор
 *		3. Создание начальной временной отметки
 *		4. Запуск полезной нагрузки главного процесса без вызова функции sched_yield() на каждой итерации полезного цикла
 *		5. Создание конечной временной отметки
 *		6. Подсчет времени, затраченного полезной нагрузкой главного процесса на выполнение
 *		7. Создание начальной временной отметки
 *		8. Запуск полезной нагрузки главного процесса с вызовом функции sched_yield() на каждой итерации полезного цикла
 *		9. Создание конечной временной отметки
 *		10. Подсчет времени, затраченного полезной нагрузкой главного процесса на выполнение
 *
 *	Теоретические результаты:
 *
 *		Пункт 10 даст большее значение времени выполнения полезной нагрузки главного процесса по сравнению с пунктом 6, так как в пункте 8
 *		свой квант процессорного времени главный процесс будет на каждой итерации полезного цикла объявлять исчерпанным (с помощью функции sched_yield())
 *		в отличии от пункта 4, в котором в один квант процессорного времени будут выполняться несколько итераций полезного цикла
 *	
 *	Компиляция:
 *
 *		gcc -lrt -lm main.c -o program.out
 *
 *		Ключ -l связывает с программой:
 *
 *			1. -lrt - часть библиотеки glibc (rt), реализующую функцию clock_gettime(), используемую процессами программы для создания временных отметок
 *			2. -lm - часть библиотеки glibc (math; m), реализующую функцию sin()
 */

#define _GNU_SOURCE			/* Для sched_setaffinity() */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <time.h>			/* Для clock_gettime() */
#include <signal.h>			/* Для kill() */
#include <sys/wait.h>		/* Для waitpid(), WIFEXITED и WIFSIGNALED */
#include <sys/types.h>		/* Для fork() */
#include <unistd.h>
#include <sched.h>			/* Для sched_yield() и sched_setaffinity() */

/* Функция set_cpu() содержит в себе программный код, устанавливающий в качестве процессора, на котором исполняются все процессы программы,
 * первый процессор системы */
int set_cpu();

/* Функция good() содержит полезную нагрузку главного процесса - вычисление полинома пятой степени для набора целых неотрицательных чисел
 * с последующим делением полученных значений на синусы чисел набора.
 *
 * Единственный параметр функции - is_sched_yield - управляет вызовом функции sched_yield() на каждой итерации полезного цикла
 * (вызовы будут выполняться в случае ненулевого значения is_sched_yield и не будут в противном случае) */
void good(const char is_sched_yield);

/* UL_NUM - количество побочный процессов, создающих бесполезную нагрузку на процессор */
#define UL_NUM 17

/* Функция ul() содержит бесполезную нагрузку бесполезных процессов */
void ul();

/* Главная функция программы */
int main()
{
	printf("\n");

	/* С помощью функции set_cpu() устанавливаем первый процессор системы как единственный,
	 * на котором должны выполнятся главный процесс программы и все его дочерние процессы */
	if(set_cpu() == -1)
	{
		perror("Ошибка при установке первого процессора системы единственным, на котором должны выполняться данный процесс и все его дочерние процессы\n");

		return -1;
	}

	int x, status, ul_pid[UL_NUM];
	
	/* Для каждого бесполезного дочернего процесса повторять... */
	for(x = 0; x < UL_NUM; x++)
		if(
			/* Запустить бесполезный дочерний процесс */
			(ul_pid[x] = fork()) == -1
		  )
			perror("Ошибка при запуске бесполезного дочернего процесса");
		else if(! ul_pid[x])
		{
			/* Мы в очередном бесполезном дочернем процессе */

			/* Запускаем бесполезную нагрузку */
			ul();

			/* Завершаем бесполезный дочерний процесс с кодом возврата 0
			 *
			 * (так как бесполезная нагрузка организована с помощью бесконечного цикла,
			 * данная строка никогда не будет достигнута) */
			exit(0);
		}

	printf("Главный процесс:\n\n");

	/* Запускаем полезную нагрузку
	 * 
	 * (функция sched_yield() на каждой итерации полезного цикла выполняться не будет) */
	good(0);

	/* Запускаем полезную нагрузку
	 * 
	 * (на каждой итерации полезного цикла будет выполняться функция sched_yield()) */
	good(1);

	/* Отправляем бесполезным дочерним процессам сигнал SIGKILL, предписывая, таким образом, ОС завершить все побочные дочерние процессы */
	for(x = 0; x < UL_NUM; x++)
		if(ul_pid[x] != -1)
			kill(ul_pid[x], SIGKILL);

	/* С помощью функции waitpid() последовательно дождемся завершения каждого из побочных дочерних процессов,
	 * проверяя при этом причину завершения - скорее всего, все побочные дочерние процессы будут завершены по сигналу
	 * SIGKILL - поэтому именно макрос WIFSIGNALED() первым вернет ненулевое значение */
	for(x = 0; x < UL_NUM; x++)
		if(ul_pid[x] != -1)
			do
				waitpid(ul_pid[x], & status, 0);
			while (! WIFEXITED(status) && ! WIFSIGNALED(status));

	return 0;
}

/* Функция set_cpu() содержит в себе программный код, устанавливающий в качестве процессора, на котором исполняются все процессы программы,
 * первый процессор системы */
int set_cpu()
{
	/* Переменная cpu_mask описывает маску процессоров, используемых процессами программы */
	cpu_set_t cpu_mask;

	/* Обнуляем маску */
	CPU_ZERO(& cpu_mask);

	/* Бит, отвечающий за первый процессор системы, устанавливаем в единицу */
	CPU_SET(0, & cpu_mask);

	/* Для данного процесса (первый параметр - 0) устанавливаем маску cpu_mask (третий параметр) размером в sizeof(cpu_mask) байт как
	 * маску используемых процессом процессоров системы. Все дочерние процессы данного процесса унаследуют данную маску */
	return sched_setaffinity(0, sizeof(cpu_mask), & cpu_mask);
}

/* Функция good() содержит полезную нагрузку главного процесса - вычисление полинома пятой степени для набора целых неотрицательных чисел
 * с последующим делением полученных значений на синусы чисел набора.
 *
 * Единственный параметр функции - is_sched_yield - управляет вызовом функции sched_yield() на каждой итерации полезного цикла
 * (вызовы будут выполняться в случае ненулевого значения is_sched_yield и не будут в противном случае) */
void good(const char is_sched_yield)
{
	int x;
	double res;
	struct timespec begin, end;

	/* Отмечаем начальную временную точку и сохраняем информацию о ней в переменной begin
	 *
	 * (CLOCK_MONOTONIC - монотонные общесистемные часы, отсчитывающие время не известно от какого момента времени в прошлом,
	 * но делающие это строго монотонно и гарантирующие, что будут также отсчитывать время и в будущем) */
	if(
		clock_gettime(CLOCK_MONOTONIC, & begin) == -1
	  )
		perror("Ошибка при создании начальной временной отметки");

	/* В зависимости от значения параметра is_sched_yield функции good() главный процесс будет (is_sched_yield != 0)
	 * или не будет (is_sched_yield == 0) вызывать sched_yield() для уступки процессора на каждой итерации полезного цикла */
	if(is_sched_yield)
		/* Для каждого целого числа из диапазона [0, 17777) вычисляем многочлен 5-й степени и делим полученный результат на синус используемого числа.
		 * На каждой итерации вызываем функцию sched_yield() для принудительной уступки процессора */
		for(x = 0; x < 177770; x++)
		{
			res = (1.2 * x * x * x * x * x -
				5.6 * x * x * x * x + 11.9867 * x * x * x -
				1577.7717 * x  * x + 87958.11 * x - 1947.22) / sin(x);
			
			/* Вызовом функции sched_yield() главный процесс уступает процессор другим процессам системы (вызовом функции sched_yield()
			 * главный процесс просит планировщик считать квант процессорного времени главного процесса исчерпанным) */
			sched_yield();
		}
	else
		/* Для каждого целого числа из диапазона [0, 17777) вычисляем многочлен 5-й степени и делим полученный результат на синус используемого числа */
		for(x = 0; x < 177770; x++)
			res = (1.2 * x * x * x * x * x -
				5.6 * x * x * x * x + 11.9867 * x * x * x -
				1577.7717 * x  * x + 87958.11 * x - 1947.22) / sin(x);

	/* Отмечаем конечную временную точку и сохраняем информацию о ней в переменной end */
	if(
		clock_gettime(CLOCK_MONOTONIC, & end) == -1
	  )
		perror("Ошибка при создании конечной временной отметки");
	else
		/* Выводим в стандартный поток вывода время в секундах, затраченное главным процессом на выполнение полезных действий
		 *
		 * Поля структуры данных timespec:
		 *
		 *		tv_sec - секунды
		 *		tv_nsec - наносекунды */
		printf("\tВремя выполнения: %.3f секунд\n\n", (end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec) / 1000000000.0);
}

/* Функция ul() содержит бесполезную нагрузку бесполезных процессов */
void ul()
{
	int x;

	/* В качестве бесполезной нагрузки - бесконечный цикл */
	while(1)
		x++;
}

