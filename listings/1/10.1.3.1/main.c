
/* Данную программу необходимо компилировать командой:
 *
 * gcc -masm=intel main.c */

#include <stdio.h>

#include <stdlib.h>		/* Для exit() */
#include <signal.h>		/* Для signal() */
#include <sys/types.h>	/* Для getpid() */
#include <unistd.h>

/* Обработчик сигнала SIGFPE, отправляющегося системой процессу в случае
 * некорректной арифметической операции */
void SIGFPE_proc(int notused)
{
	fprintf(stderr, "\nПришел сигнал SIGFPE\n\n");

	/* Завершаем работу программы */
	exit(0);
}

int main()
{
	/* Устанавливаем действия по получении сигналов:
	 *
	 * SIGFPE - функция - обработчик SIGFPE_proc();
	 * SIGTERM - игнорировать*/
	if(
			signal(SIGFPE, SIGFPE_proc) == SIG_ERR
			||
			signal(SIGTERM, SIG_IGN) == SIG_ERR
	  )
		perror("Ошибка при установке обработчика сигнала");

	/* Отправляем сигнал SIGTERM. Он будет проигнорирован и процесс не будет завершен */
	if(
			kill(getpid(), SIGTERM)
	  )
		perror("Ошибка при отправлении сигнала SIGTERM");

	printf("Деление на ноль\n");

	/* Производим деление на ноль - система должна отправить процессу сигнал SIGFPE
	 * (для разнообразия - реализуем деление на ноль на ассемблере) */
	__asm(".intel_syntax noprefix\n\
    mov eax,0xFFCCAA\n\
	xor edx,edx\n\
	xor ebx,ebx\n\
	div ebx");

	return 0;
}

