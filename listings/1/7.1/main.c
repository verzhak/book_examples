
#include <stdio.h>

#include <stdlib.h>			/* Для lrand48() */
#include <sys/time.h>		/* Для getrusage() */
#include <sys/resource.h>

int main()
{
	printf("\n");

	int x;
	long a;
	double b, c;

	/* Занимаем процессор различной бессмысленной работой */
	for(x = 0; x < 1024563; x ++)
	{
		a = lrand48();
		b = a / (lrand48() * 12.0) + 5478 * b - c;
		c = (a % 1283) / 0.947582;
		b = a / (lrand48() * 12.0) + 5478 * b - c;
		c = (a % 1283) / 0.947582;
		b = a / (lrand48() * 12.0) + 5478 * b - c;
		c = (a % 1283) / 0.947582;
	}

	struct rusage usage;

	/* Получаем информацию об используемых процессом ресурсах */
	if(
		getrusage(RUSAGE_SELF, & usage) == -1
	  )
		perror("Ошибка при получении информации об используемых процессом ресурсах");
	else
	{
		printf("Затраченное процессом процессорное время:\n");
		printf("\tВ пользовательском пространстве:  %ld секунды\n",	usage.ru_utime.tv_sec);
		printf("\tВ пространстве ядра:  %ld секунд\n", usage.ru_stime.tv_sec);
	}

	printf("\n");

	return 0;
}

