
#include <stdio.h>

#include <stdlib.h>		/* Для system() */

int main()
{
	printf("\nКоманда echo вернула код завершения = %d\n\n",
			system("echo -e '\\nТестируем system()'"));

	return 0;
}

