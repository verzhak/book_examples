
/* Данная программа суть есть пример использования функций chown() и lchown() для изменения
 * группы пользователей целевого файла
 *
 * Алгоритм работы программы:
 *
 *		1. Определяем идентификатор текущей группы пользователей целевого файла
 *		2. Определяем идентификатор владельца целевого файла
 *		3. По идентификатору владельца определяем имя владельца целевого файла
 *		4. По имени владельца целевого файла получаем список идентификаторов групп пользователей,
 *		   в которых состоит владелец целевого файла
 *		5. Изменяем группу пользователей целевого файла на первую группу пользователей из списка, полученного
 *		   в пункте 4, идентификатор которой отличен от идентификатора группы, полученного в пункте 1
 *		6. Изменяем группу пользователей целевого файла на группу, идентификатор которой получен в пункте 1
 */

#include <stdio.h>

#include <grp.h>		/* Для getgrouplist() */
#include <pwd.h>		/* Для getpwuid() */
#include <stdlib.h>		/* Для system() */
#include <sys/types.h>	/* Для stat() */
#include <sys/stat.h>
#include <unistd.h>		/* Для chown() и lchown() */

int main()
{
	printf("\n");

	struct stat buf;

	/* Получаем метаданные файла ./program.out - нас интересуют идентификаторы владельца
	 * и группы пользователей файла (buf.st_uid и buf.st_gid соответственно) */
	if(
		stat("./program.out", & buf) == -1
	  )
	{
		perror("Ошибка при получении атрибутов файла ./program.out");

		return -1;
	}

	/* Получаем имя пользователя (поле pw_name структуры данных passwd)
	 * по его идентификатору (единственный параметр функции getpwuid()) */
	struct passwd *user = getpwuid(buf.st_uid);

	if(user == NULL)
	{
		perror("Ошибка при получении имени владельца файла ./program.out");

		return -1;
	}	

	gid_t groups[100];
	int ngroups = 100;

	/* Получаем идентификаторы групп пользователей (будут возвращены в массиве,
	 * на который указывает третий аргумент функции getgrouplist()), в которые
	 * входит пользователь с именем, переданным в первом аргументе данной функции */
	if(
		getgrouplist(user->pw_name, buf.st_gid, groups, & ngroups) < 2
	  )
	{
		perror("Ошибка при получении идентификаторов хотя бы двух групп, в которых состоит владелец файла ./program.out");

		return -1;
	}

	/* Определяем идентификатор новой группы целевого файла - это значение первого элемента массива
	 * groups, если идентификатор предыдущей группы совпадает со значением нулевого элемента,
	 * или значение нулевого элемента - в противном случае */
	int x = (groups[0] == buf.st_gid) ? 1 : 0;

	/* С помощью утилиты ls выводим имя текущей группы файла ./program.out */
	system("ls -l program.out");

	printf("\nИзменить группу пользователей файла ./program.out\n\n");

	/* Изменяем группу пользователей файла ./program.out (владельца файла не трогаем, передавая
	 * в chown() в качестве второго параметра -1) */
	if(
		chown("./program.out", -1, groups[x]) == -1
	  )
		printf("Ошибка при изменении группы пользователей файла ./program.out");
	else
		/* С помощью утилиты ls выводим имя текущей группы файла ./program.out */
		system("ls -l program.out");

	printf("\nУстановить прежную группу пользователей файла ./program.out\n\n");

	/* Изменяем группу пользователей файла ./program.out (владельца файла не трогаем, передавая
	 * в lchown() в качестве второго параметра -1) */
	if(
		lchown("./program.out", -1, buf.st_gid) == -1
	  )
		printf("Ошибка при изменении группы пользователей файла ./program.out");
	else
		/* С помощью утилиты ls выводим имя текущей группы файла ./program.out */
		system("ls -l program.out");

	printf("\n");

	return 0;
}

