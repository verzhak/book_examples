
#include <stdio.h>
#include <stdlib.h>

#include <errno.h>	/* Для errno */
#include <time.h>	/* Для nanosleep(), time() и ctime() */

int main()
{
	printf("\n");

	int x;
	struct timespec tm, signal_tm;

	/* С помощью функции time() получаем количество секунд, прошедших с начала эпохи (1 января 1970 года)
	 * и преобразуем (с помощью функции ctime()) полученное значение в строку, содержащую информацию о текущей дате и времени дня */
	time_t tm_past = time(NULL);
	printf("Текущее время: %s\n", ctime(& tm_past));

	/* Пять раз приостановим выполнение процесса на секунду */
	for(x = 0; x < 5; x++)
	{
		/* Случайным образом генерируем временной интервал, на который будет усыплен процесс */
		tm.tv_sec = rand() % 5;				/* Секунды */
		tm.tv_nsec = rand() % 1000000000;	/* Наносекунды (вряд ли ОС способна обеспечить точность до наносекунд,
											   максимальная точность - в лучшем случае - миллисекунды) */

		printf("---> Приостанавливаем выполнение процесса на %.9f секунд <---\n\n",
				tm.tv_sec + tm.tv_nsec / 1000000000.0);

		/* Приостанавливаем выполнение процесса на время, описанное первым параметром nanosleep().
		 * В своем втором параметре функция nanosleep() вернет количество секунд и наносекунд, оставшееся от
		 * требуемого интервала в случае, если сон процесса был прерван неблокируемым сигналом */
		if(
			nanosleep(& tm, & signal_tm) == -1
		  )
		{
			/* Если errno установлена в EINTR, то сон процесса прерван пришедшим неблокируемым сигналом -
			 * в этом случае выведем в стандартный поток вывода информацию об оставшемся от первоначального времени сна времени */
			if(errno == EINTR)
				printf("Засыпание прервано сигналом (осталось %.9f секунд)\n",
					signal_tm.tv_sec + signal_tm.tv_nsec / 1000000000.0);
			else
				perror("Ошибка при засыпании процесса");
		}

		/* С помощью функции time() получаем количество секунд, прошедших с начала эпохи (1 января 1970 года)
		 * и преобразуем (с помощью функции ctime()) полученное значение в строку, содержащую информацию о текущей дате и времени дня */
		tm_past = time(NULL);
		printf("Текущее время: %s\n", ctime(& tm_past));
	}

	return 0;
}

