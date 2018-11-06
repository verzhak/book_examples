
#include <stdio.h>

#include <sys/types.h>	/* Для getpid() и getppid() */
#include <unistd.h>

int main()
{
	printf("\nPID = %d\nРодительский PID = %d\n\n",
			getpid(),
			getppid()
		  );

	return 0;
}

