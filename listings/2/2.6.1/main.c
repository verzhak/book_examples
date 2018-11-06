
#include <stdio.h>

#include <unistd.h>		/* Для getcwd() */

int main()
{
	char buf[1024];

	/* Получаем полный путь и имя текущего каталога
	 *
	 * (после успешного выполнения getcwd() значения ret и buf должны быть
	 * равны (то есть оба указателя должны указывать на один и тот же буфер)) */
	char *ret = getcwd(buf, 1024);

	if(ret == NULL)
		perror("Ошибка при определении текущего каталога");
	else
		printf("\nТекущий каталог:\n\t%s\n\t%s\n\n", buf, ret);

	return 0;
}

