
#define _GNU_SOURCE		/* Для O_DIRECTORY */

#include <stdio.h>

#include <unistd.h>		/* Для chdir(), fchdir(), close(), getcwd() */
#include <sys/types.h>	/* Для open() */
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
	printf("\n");

	char buf[1024];

	if(
		/* Получаем информацию о текущем каталоге */
		getcwd(buf, 1024) == NULL
	  )
		perror("Ошибка при определении текущего каталога");
	else
		printf("Текущий каталог: %s\n", buf);
	
	if(
		/* Переходим в корневой каталог */
		chdir("/") == -1
	  )
		perror("Ошибка при переходе в корневой каталог");
	else if(
			/* Если переход в корневой каталог оказался успешным, то
			 * определим текущий каталог процесса (очевидно, им является /) */
			getcwd(buf, 1024) == NULL
		   )
		perror("Ошибка при определении текущего каталога");
	else
		printf("Текущий каталог: %s\n", buf);
	
	/* Открываем каталог /var, используя функцию open() и флаг O_DIRECTORY */
	int fd = open("/var", O_RDONLY | O_DIRECTORY);

	if(fd == -1)
		perror("Ошибка при открытии каталога /var");
	else
	{
		/* Каталог /var успешно открыт */

		if(
			/* Переходим в каталог /var */
			fchdir(fd) == -1
		  )
			perror("Ошибка при переходе в каталог /var");
		else if(
				/* Если переход в каталог /var оказался успешным, то
				 * определим текущий каталог процесса (очевидно, это каталог /var) */
				getcwd(buf, 1024) == NULL
			   )
			perror("Ошибка при определении текущего каталога");
		else
			printf("Текущий каталог: %s\n", buf);

		/* Закрываем каталог /var */
		close(fd);
	}

	printf("\n");

	return 0;
}

