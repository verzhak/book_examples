
/* Алгоритм работы программы:
 *
 *		1. Явная привязка всех процессов программы к строго определенному процессору системы с помощью функции sched_setaffinity() -
 *		   необходимо для многопроцессорных и многоядерных систем для предотвращения распределения создаваемой процессами программы нагрузки
 *		2. Запуск UL_NUM бесполезных процессов, создающих нагрузку на процессор
 *		3. Создание начальной временной отметки
 *		4. Запуск полезного дочернего процесса (выполняет расчет многочлена пятой степени для набора целых неотрицательных чисел и
 *		   делит полученные результаты на синусы данных чисел)
 *		5. Ожидание завершения полезного дочернего процесса
 *		6. Создание конечной временной отметки
 *		7. Подсчет времени, затраченного полезным дочерним процессом на выполнение
 *
 *		Пункты 3 - 7 повторяются для различных значений любезности дочернего процесса
 *
 *	Процессами программы используется политика планирования SCHED_OTHER (политика планирования по умолчанию),
 *	подразумевающая распределение процессорного времени с помощью значений любезности
 *	
 *	Теоретические результаты:
 *
 *		Любезность -20 даст наименьшее время выполнения полезного дочернего процесса, тогда как любезность 19 даст наибольшее время выполнения
 *		полезного дочернего процесса. В общем случае, должна быть отслежена следующая зависимость: с увеличением любезности полезного дочернего процесса
 *		увеличивается его время выполнения
 *	
 *	Компиляция:
 *
 *		gcc -lrt -lm main.c -o program.out
 *
 *		Ключ -l связывает с программой:
 *
 *			1. -lrt - часть библиотеки glibc (rt), реализующую функцию clock_gettime(), используемую процессами программы для создания временных отметок
 *			2. -lm - часть библиотеки glibc (math; m), реализующую функцию sin()
 *
 *	Запуск:
 *
 *		su -c './program.out'
 *
 *		(запускать программу необходимо от имени суперпользователя, так как процессы, имеющие действительным владельцем суперпользователя, имеют характеристику
 *		CAP_SYS_NICE, необходимую для использования отрицательных значений любезности)
 */

#define _GNU_SOURCE			/* Для sched_setaffinity() */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <sched.h>			/* Для sched_setaffinity() */
#include <time.h>			/* Для clock_gettime() */
#include <unistd.h>			/* Для fork() */
#include <sys/types.h>
#include <signal.h>			/* Для kill() */
#include <sys/wait.h>		/* Для waitpid(), WIFEXITED и WIFSIGNALED */
#include <errno.h>			/* Для errno */
#include <sys/time.h>		/* Для setpriority() и getpriority() */
#include <sys/resource.h>

/* Функция set_cpu() содержит в себе программный код, устанавливающий в качестве процессора, на котором исполняются все процессы программы,
 * первый процессор системы */
int set_cpu();

/* Функция good() содержит полезную нагрузку полезного дочернего процесса - вычисление полинома пятой степени для набора целых неотрицательных чисел
 * с последующим делением полученных значений на синусы чисел набора */
void good();

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
		perror("Ошибка при установке первого процессора системы единственным, на котором должны выполняться данный процесс и все его дочерние процессы");

		return -1;
	}

	int x, status, good_pid, ul_pid[UL_NUM];

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

	/* Мы в родительском процессе */

	printf("Полезный дочерний процесс:\n\n");
	
	/* Для значений любезности в диапазоне от -20 до 19 с шагом 3 последовательно запустим полезные дочерние процессы,
	 * установим значение любезности запущенного полезного дочернего процесса в значение переменной x
	 * и произведем замер времени выполнения данных дочерних процессов */
	for(x = -20; x < 20; x += 3)
		/* Запускаем полезный дочерний процесс */
		if(
			(good_pid = fork()) == -1
		  )
			perror("Ошибка при запуске полезного дочернего процесса");
		else if(good_pid)
			/* Мы в родительском процессе
			 *
			 * Ожидаем с помощью функции waitpid() завершение (макрос WIFEXITED() возвращает 1,
			 * если переданная ему информация о статусе процесса сообщает о произошедшем завершении процесса) запущенного
			 * дочернего процесса, PID которого передан в waitpid() в качестве первого параметра - good_pid */
			do
				waitpid(good_pid, & status, 0);
			while (! WIFEXITED(status));
		else
		{
			/* Мы в полезном дочернем процессе */

			int nice_value;
			struct timespec begin, end;

			/* Обнуляем переменную errno
			 *
			 * Если значение errno не изменится после вызова функции getpriority(), то это будет означать,
			 * что данная функция завершилась успешно - надеяться на возврат getpriority() -1 только в случае ошибки некорректно,
			 * так как данная функция может вернуть -1 и в случае успеха - в том случае, если значение любезности процесса равно -1 */ 
			errno = 0;

			/* Устанавливаем любезность вызывающего (полезного дочернего; в качестве второго параметра в setpriority() передан 0)
			 * процесса (указано PRIO_PROCESS) в значение, передаваемое в setpriority() в качестве третьего параметра (x) */
			if(
				setpriority(PRIO_PROCESS, 0, x) == -1
			  )
				perror("Ошибка при изменении значения любезности полезного дочернего процесса");
			/* Получаем значение любезности вызывающего (полезного дочернего; в качестве второго параметра в getpriority() передан 0)
			 * процесса (указано PRIO_PROCESS) */
			else if(
					(nice_value = getpriority(PRIO_PROCESS, 0)) == -1
					&&
					errno
					)
				perror("Ошибка при получении значения любезности полезного дочернего процесса");
			/* Отмечаем начальную временную точку и сохраняем информацию о ней в переменной begin
			 *
			 * (CLOCK_MONOTONIC - монотонные общесистемные часы, отсчитывающие время не известно от какого момента времени в прошлом,
			 * но делающие это строго монотонно и гарантирующие, что будут также отсчитывать время и в будущем) */
			else if(
				clock_gettime(CLOCK_MONOTONIC, & begin) == -1
			  )
				perror("Ошибка при создании начальной временной отметки");
			else
			{
				/* Выводим в стандартный поток вывода текущее значение любезности полезного дочернего процесса */
				printf("\tЛюбезность: %d\n", nice_value);

				/* Запускаем на выполнение полезную нагрузку полезного дочернего процесса */
				good();

				/* Отмечаем конечную временную точку и сохраняем информацию о ней в переменной end */
				if(
					clock_gettime(CLOCK_MONOTONIC, & end) == -1
				  )
					perror("Ошибка при создании конечной временной отметки");
				else
					/* Выводим в стандартный поток вывода время в секундах, затраченное полезным дочерним процессом на выполнение полезных действий
					 *
					 * Поля структуры данных timespec:
					 *
					 *		tv_sec - секунды
					 *		tv_nsec - наносекунды */
					printf("\tВремя выполнения: %.3f секунд\n\n", (end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec) / 1000000000.0);
			}

			/* Завершаем полезный дочерний процесс с кодом завершения 0 */
			exit(0);
		}

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

/* Функция good() содержит полезную нагрузку полезного дочернего процесса - вычисление полинома пятой степени для набора целых неотрицательных чисел
 * с последующим делением полученных значений на синусы чисел набора */
void good()
{
	double res;
	unsigned long x;

	/* Для каждого целого числа из диапазона [0, 17777) вычисляем многочлен 5-й степени и делим полученный результат на синус используемого числа */
	for(x = 0; x < 17777; x++)
		res = (1.2 * x * x * x * x * x -
			5.6 * x * x * x * x + 11.9867 * x * x * x -
			1893.14683 * x  * x + 87958.11 * x - 1947.22) / sin(x);
}

/* Функция ul() содержит бесполезную нагрузку бесполезных процессов */
void ul()
{
	int x;

	/* В качестве бесполезной нагрузки - бесконечный цикл */
	while(1)
		x++;
}

