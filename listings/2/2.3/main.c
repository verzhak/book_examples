
#include <stdio.h>

#include <stdlib.h>		/* Для system() */
#include <unistd.h>		/* Для close() */
#include <sys/types.h>	/* Для open() */
#include <fcntl.h>
#include <sys/stat.h>	/* Для fchmod() и chmod() */

int main()
{
	printf("\n");

	/* Открываем файл ./program.out на чтение - для использования функции fchmod() нам необходим
	 * номер открытого файлового дескриптора данного файла */
	int fd = open("./program.out", O_RDONLY);

	if(fd == -1)
	{
		perror("Ошибка при открытии файла ./program.out на чтение");

		return -1;
	}

	/* С помощью утилиты ls выводим данные о первоначальных правах доступа пользователей к файлу ./program.out */
	system("ls -l program.out");

	printf("\nУстановить маску прав доступа: rw------- на файл ./program.out\n\n");

	/* Устанавливаем маску прав доступа к файлу ./program.out в значение rw-------
	 * (владелец имеет права чтения и записи, прочие пользователи не имеют
	 * никаких прав доступа к файлу) */
	if(
		fchmod(fd, S_IRUSR | S_IWUSR) == -1
	  )
		perror("Ошибка при изменении маски прав доступа к файлу ./program.out");
	else
		/* С помощью утилиты ls выводим данные о правах доступа пользователей к файлу ./program.out */
		system("ls -l program.out");

	/* Закрываем файловый дескриптор */
	close(fd);

	printf("\nУстановить маску прав доступа: rwx------ на файл ./program.out\n\n");

	/* Устанавливаем маску прав доступа к файлу ./program.out в значение rwx------
	 * (владелец имеет права чтения, записи и выполнения, прочие пользователи не имеют
	 * никаких прав доступа к файлу) */
	if(
		chmod("./program.out", S_IRWXU) == -1
	  )
		perror("Ошибка при изменении маски прав доступа к файлу ./program.out");
	else
		/* С помощью утилиты ls выводим данные о правах доступа пользователей к файлу ./program.out */
		system("ls -l program.out");

	printf("\n");

	return 0;
}

