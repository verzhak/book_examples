
#include <stdio.h>

#include <unistd.h>			/* Для getuid(), getgid(), geteuid() и getegid() */
#include <sys/types.h>

int main()
{
	/* Получаем реальный идентификатор пользователя процесса */
	uid_t uid = getuid();
	/* Получаем реальный идентификатор группы пользователей процесса */
	gid_t gid = getgid();

	/* Получаем действительный (эффективный) идентификатор пользователя процесса */
	uid_t euid = geteuid();
	/* Получаем действительный (эффективный) идентификатор группы пользователей процесса */
	uid_t egid = getegid();

	printf("\nИдентификаторы владельцев и групп пользователей процесса:\n\n");
	printf("\tРеальный идентификатор пользователя\t\t=\t%u\n", uid);
	printf("\tРеальный идентификатор группы пользователей\t=\t%u\n", gid);
	printf("\tДействительный (эффективный) идентификатор пользователя\t\t\t=\t%u\n", euid);
	printf("\tДействительный (эффективный) идентификатор группы пользователей\t\t=\t%u\n\n", egid);

	return 0;
}

