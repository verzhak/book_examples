
#include <stdio.h>
#include <stdlib.h>

#include <time.h>		/* Для time() и ctime() */
#include <signal.h>		/* Для signal() */
#include <sys/time.h>	/* Для setitimer() и getitimer() */

/* Функция - обработчик сигналов SIGALRM, SIGVTALRM и SIGPROF */
void SIG_proc(int sig)
{
	/* Статическая переменная sc подсчитывает количество пришедших процессу сигналов */
	static int sc;

	/* Обрабатываем пришедший сигнал */
	switch(sig)
	{
		case SIGALRM:
			{
				/* Пришел сигнал SIGALRM - сработал таймер ITIMER_REAL */

				printf("Процесс получил сигнал SIGALRM\t\t-\tсработал таймер ITIMER_REAL");
				break;
			}
		case SIGVTALRM:
			{
				/* Пришел сигнал SIGVTALRM - сработал таймер ITIMER_VIRTUAL */

				printf("Процесс получил сигнал SIGVTALRM\t-\tсработал таймер ITIMER_VIRTUAL");
				break;
			}
		case SIGPROF:
			{
				/* Пришел сигнал SIGPROF - сработал таймер ITIMER_PROF */

				printf("Процесс получил сигнал SIGPROF\t\t-\tсработал таймер ITIMER_PROF");
				break;
			}
	}

	/* С помощью функции time() получаем количество секунд, прошедших с начала эпохи (1 января 1970 года)
	 * и преобразуем (с помощью функции ctime()) полученное значение в строку, содержащую информацию о текущей дате и времени дня */
	time_t tm = time(NULL);
	printf("\n\tТекущее время: %s\n", ctime(& tm));

	/* Процесс будет завершен по получении семи сигналов от таймеров */
	if(sc == 7)
		exit(0);

	sc++;
}

/* Главная функция программы */
int main()
{
	printf("\n");

	/* Устанавливаем функцию SIG_proc() в качестве функции - обработчика сигналов
	 * SIGALRM, SIGVTALRM и SIGPROC, получаемых по срабатыванию таймеров
	 * ITIMER_REAL, ITIMER_VIRTUAL и ITIMER_PROF соответственно */
	if(
		signal(SIGALRM, & SIG_proc) == SIG_ERR
		||
		signal(SIGVTALRM, & SIG_proc) == SIG_ERR
		||
		signal(SIGPROF, & SIG_proc) == SIG_ERR
	  )
		perror("Ошибка при установке обработчиков сигналов SIGALRM, SIGVTALRM и SIGPROF");
	else
	{
		struct itimerval real_tm, virtual_tm, prof_tm;
		struct itimerval real_tm_gt, virtual_tm_gt, prof_tm_gt;

		/* Случайным образом заполняем экземпляры структуры данных itimerval -
		 * real_tm, virtual_tm и prof_tm, хранящие временные интервалы запускаемых таймеров */

		real_tm.it_value.tv_sec = rand() % 5;
		real_tm.it_value.tv_usec = rand() % 1000000;
		real_tm.it_interval.tv_sec = rand() % 5;
		real_tm.it_interval.tv_usec = rand() % 1000000;

		virtual_tm.it_value.tv_sec = rand() % 5;
		virtual_tm.it_value.tv_usec = rand() % 1000000;
		virtual_tm.it_interval.tv_sec = rand() % 5;
		virtual_tm.it_interval.tv_usec = rand() % 1000000;

		prof_tm.it_value.tv_sec = rand() % 5;
		prof_tm.it_value.tv_usec = rand() % 1000000;
		prof_tm.it_interval.tv_sec = rand() % 5;
		prof_tm.it_interval.tv_usec = rand() % 1000000;

		/* С помощью функции time() получаем количество секунд, прошедших с начала эпохи (1 января 1970 года)
		 * и преобразуем (с помощью функции ctime()) полученное значение в строку, содержащую информацию о текущей дате и времени дня */
		time_t tm = time(NULL);
		printf("Текущее время: %s\n", ctime(& tm));

		/* Запускаем таймеры ITIMER_REAL, ITIMER_VIRTUAL и ITIMER_PROF
		 *
		 * (в качестве второго параметра в функцию setitimer() передается информация о временных интервалах запускаемых таймеров) */
		if(
			setitimer(ITIMER_REAL, & real_tm, NULL) == -1
			||
			setitimer(ITIMER_VIRTUAL, & virtual_tm, NULL) == -1
			||
			setitimer(ITIMER_PROF, & prof_tm, NULL) == -1
		  )
			perror("Ошибка при запуске одного из таймеров");
		else
		{
			/* Получаем временные интервалы (возвращаются во втором параметре функции getitimer())
			 * таймеров ITIMER_REAL, ITIMER_VIRTUAL и ITIMER_PROF */
			if(
				getitimer(ITIMER_REAL, & real_tm_gt) == -1
				||
				getitimer(ITIMER_VIRTUAL, & virtual_tm_gt) == -1
				||
				getitimer(ITIMER_PROF, & prof_tm_gt) == -1
			  )
				perror("Ошибка при получении информации об одном из таймеров");
			else
			{
				/* Выводим в стандартный поток вывода временные интервалы (возвращаются во втором параметре функции getitimer()) таймеров
				 * ITIMER_REAL, ITIMER_VIRTUAL и ITIMER_PROF */

				printf("Таймер ITIMER_REAL:\n\n\tВременной интервал до первого тика таймера = %.6f\n\tВременной интервал между тиками таймера = %.6f\n\n",
						real_tm_gt.it_value.tv_sec + real_tm_gt.it_value.tv_usec / 1000000.0,
						real_tm_gt.it_interval.tv_sec + real_tm_gt.it_interval.tv_usec / 1000000.0);
				
				printf("Таймер ITIMER_VIRTUAL:\n\n\tВременной интервал до первого тика таймера = %.6f\n\tВременной интервал между тиками таймера = %.6f\n\n",
						virtual_tm_gt.it_value.tv_sec + virtual_tm_gt.it_value.tv_usec / 1000000.0,
						virtual_tm_gt.it_interval.tv_sec + virtual_tm_gt.it_interval.tv_usec / 1000000.0);

				printf("Таймер ITIMER_PROF:\n\n\tВременной интервал до первого тика таймера = %.6f\n\tВременной интервал между тиками таймера = %.6f\n\n",
						prof_tm_gt.it_value.tv_sec + prof_tm_gt.it_value.tv_usec / 1000000.0,
						prof_tm_gt.it_interval.tv_sec + prof_tm_gt.it_interval.tv_usec / 1000000.0);
			}

			/* Бесконечный цикл процесса (будет прерван вызовом функции exit() в функции SIG_proc() после получения процессом семи сигналов) */
			while(1);
		}
	}

	return 0;
}

