
#include <stdio.h>

#include <unistd.h>		/* Для sysconf(), _SC_PAGESIZE и getpagesize() */

int main()
{
	/* Получаем размер страницы с помощью функции sysconf(), передав ей в качестве параметра константу компилятора _SC_PAGESIZE */
	long page_size_sysconf = sysconf(_SC_PAGESIZE);

	/* Получаем размер страницы с помощью функции getpagesize() */
	size_t page_size_getpagesize = getpagesize();

	printf("\nРазмер страницы:\n\n\tПо версии sysconf():\t\t%ld байт\n\n\tПо версии getpagesize():\t%d байт\n\n",
			page_size_sysconf, page_size_getpagesize);

	return 0;
}

