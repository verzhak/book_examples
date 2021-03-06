
/* Для успешного выполнения операций установки реальных и действительных (эффективных) идентификаторов
 * пользователей и групп пользователей процесса процесс должен обладать характеристиками CAP_SETUID и CAP_SETGID -
 * рекомендуется запускать программу от имени суперпользователя командой "su -c './program.out'" */

#define _GNU_SOURCE			/* Для getresuid() и getresgid() */

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>			/* Для setreuid(), setregid(), getresuid() и getresgid() */
#include <sys/types.h>

/* Функция print_re() выводит на экран значения реальных и действительных (эффективных) идентификаторов
 * пользователей и групп пользователей процесса */
void print_re();

int main()
{
	/* Выводим на экран значения реальных и действительных (эффективных) идентификаторов
	 * пользователей и групп пользователей процесса */
	print_re();

	printf("\n---> Изменение реальных и действительных (эффективных) идентификаторов пользователей и групп пользователей процесса\n");

	if(
		/* Устанавливаем в качестве реального и действительного (эффективного) идентификаторов групп пользователей процесса
		 * случайные целые в диапазоне от 1 до 3 */
		setregid(rand() % 3 + 1, rand() % 3 + 1) == -1
		||
		/* Устанавливаем в качестве реального и действительного (эффективного) идентификаторов пользователей процесса
		 * случайные целые в диапазоне от 1 до 3 */
		setreuid(rand() % 3 + 1, rand() % 3 + 1) == -1
	  )
		/* Произошла ошибка EPERM - процесс не обладает одной из характеристик: CAP_SETUID или CAP_SETGID */
		perror("\nОшибка при установке реальных и действительных (эффективных) идентификаторов пользователей и групп пользователей процесса");
	else
		/* Установка реальных и действительных (эффективных) идентификаторов прошла успешно
		 *
		 * Выводим на экран значения реальных и действительных (эффективных) идентификаторов
		 * пользователей и групп пользователей процесса */
		print_re();

	printf("\n");

	return 0;
}

/* Функция print_re() выводит на экран значения реальных и действительных (эффективных) идентификаторов
 * пользователей и групп пользователей процесса */
void print_re()
{
	uid_t ruid, euid, notused_u;
	gid_t rgid, egid, notused_g;
	
	printf("\nИдентификаторы владельцев и групп пользователей процесса:\n\n");

	/* Получаем реальный и действительный (эффективный) идентификаторы пользователей процесса */
	if(
		getresuid(& ruid, & euid, & notused_u) == -1
	  )
		perror("Ошибка при получении реального и действительного (эффективного) идентификаторов пользователей процесса");
	else
		printf("\tРеальный идентификатор пользователя\t\t\t\t\t=\t%u\n\
\tДействительный (эффективный) идентификатор пользователя\t\t\t=\t%u\n", ruid, euid);

	printf("\n");
	
	/* Получаем реальный и действительный (эффективный) идентификаторы групп пользователей процесса */
	if(
		getresgid(& rgid, & egid, & notused_g) == -1
	  )
		perror("Ошибка при получении реального и действительного (эффективного) идентификаторов групп пользователей процесса");
	else
		printf("\tРеальный идентификатор группы пользователей\t\t\t\t=\t%u\n\
\tДействительный (эффективный) идентификатор группы пользователей\t\t=\t%u\n", rgid, egid);
}

