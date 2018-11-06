
/* Дочерний процесс - менеджер 4-x стеков
 * Родительский процесс - клиент дочернего
 *
 * Протокол:
 *
 *		############################ PUSH ############################
 *		
 *		Описание:
 *			затолкнуть в определенный стек 30-и битовое целое
 *		
 *		Команда:
 *			сигнал SIGUSR1 с полезной нагрузкой: |__|______________________________|
 *											     31  29                            0
 *								   номер стека___|	 |_____30-и битовое  целое_____|
 *		
 *		Ответ:
 *			успех - сигнал SIGUSR1 с полезной нагрузкой 0xFFFFFFFF
 *			неудача - сигнал SIGUSR1 с полезной нагрузкой 0
 *		
 *		############################ POP  ############################
 *		
 *		Описание:
 *			извлечь 30-и битовое целое число из вершины стека
 *		
 *		Команда:
 *			сигнал SIGUSR2 с полезной нагрузкой: |__|000000000000000000000000000000|
 *												 31  29                            0
 *									номер стека__|
 *		
 *		Ответ:
 *			успех - сигнал SIGUSR1 с полезной нагрузкой: |11|______________________________|
 *													     31  29                            0
 *															 |_____30-и битовое  целое_____|
 *			неудача - сигнал SIGUSR1 с полезной нагрузкой 0
 *
 *		############################ INIT ############################
 *		
 *		Описание:
 *			инициализация стеков
 *		
 *		Команда:
 *			сигнал SIGHUP
 *
 *		############################ DEST ############################
 *		
 *		Описание:
 *			уничтожение стеков, завершение менеджера стеков
 *		
 *		Команда:
 *			сигнал SIGTERM
 *
 *		############################################################## */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>			/* Для uint32_t */

#include <unistd.h>			/* Для pause() */
#include <signal.h>			/* Для sigqueue(), sigaction(), kill() */
#include <sys/types.h>		/* Для kill() */
#include <wait.h>			/* Для waitpid() */

/* ############################################################################
 *								Дочерний процесс							 */

#define STACK_NUM 4			/* Количество стеков (изменять без изменения протокола невозможно) */
#define STACK_SIZE 4096		/* Размер стека (в байтах; необходимо, чтобы был кратен 4-м) */

/* Описатель стека и массив описателей стеков */
struct s_stack
{
	/* m — указатель на начало области памяти, отведенной под стек
	 * head — вершина стека */
	uint32_t *m, *head;

	/* Флаг, установлен в 1, если стек не пуст. Если стек пуст - установлен в 0 */
	uint8_t is_not_empty;
} *stack = NULL;

void push(siginfo_t *si);	/* Обработать PUSH, пришедший сигналом SIGUSR1 */
void pop(siginfo_t *si);	/* Обработать POP, пришедший сигналом SIGUSR2 */
void init();				/* Инициализация по сигналу SIGHUP */
void destroy();				/* Завершение работы менеджера стеков по сигналу SIGTERM */

/* Основной обработчик сигналов менеджера стеков */

void manager_sig_proc(int signum, siginfo_t *si, void *notused)
{
	/* В соответствии с принятым сигналом вызвать нужную функцию */
	switch(signum)
	{
		case SIGUSR1:
			{
				push(si);
				break;
			}
		case SIGUSR2:
			{
				pop(si);
				break;
			}
		case SIGHUP:
			{
				init();
				break;
			}
		case SIGTERM:
			{
				destroy();
				break;
			}
	}
}

/* Команда PUSH
 *
 * (номер стека и значение в si->si_int) */

void push(siginfo_t *si)
{
	/* Получаем указатель на стек - из массива стеков по индексу, равному двум первым
	 * битам si->si_int (в данном случае (ia32; особенности siginfo_t) sizeof(int) == 4) */
	struct s_stack *st = & stack[(si->si_int & 0xC0000000) >> 30];

	/* Значение ret будет возвращено процессу, отправившему PUSH */
	uint32_t ret = 0;

	/* Проверяем, есть ли в стеке место */
	if((st->head - st->m) < STACK_SIZE)
	{
		/* Да, есть */

		/* Получаем значение, которое необходимо поместить в стек
		 *
		 * (младшие 30 битов si->si_int) */
		ret = si->si_int & 0x3FFFFFFF;

		if(st->is_not_empty)
			/* Если стек не пуст - инкрементируем указатель вершины (на самом деле значение увеличится на 4 байта, так как
			 * указатель вершины имеет тип uint32_t - целое 4-х байтовое беззнаковое), после чего записываем в вершину
			 * значение
			 *
			 * (в силу приоритетов операций: (++st->head) выполнится до присваивания, в отличие от (st->head++)) */
			*(++st->head) = ret;
		else
		{
			/* Стек пуст - записываем значение в вершину стека */
			st->is_not_empty = 1;
			*(st->head) = ret;
		}

		/* Возвращать процессу, сделавшему PUSH, необходимо 0xFFFFFFFF */
		ret = ~ ((uint32_t) 0);

	}

	union sigval sv;

	sv.sival_int = (int) ret;

	/* Сигналом SIGUSR1 возвращаем процессу, сделавшему PUSH (PID процесса в si->si_pid),
	 * код возврата (ret): 0 в случае ошибки, 0xFFFFFFFF в случае успеха */
	if(
			sigqueue(si->si_pid, SIGUSR1, sv) == -1
	  )
		perror("Ошибка отправления ответа на push");
}

/* Команда POP
 *
 * (номер стека в si->si_int) */

void pop(siginfo_t *si)
{
	/* Получаем указатель на стек - из массива стеков по индексу, равному двум первым
	 * битам si->si_int (в данном случае (ia32; особенности siginfo_t) sizeof(int) == 4) */
	struct s_stack *st = & stack[(si->si_int & 0xC0000000) >> 30];

	/* Значение ret будет возвращено процессу, отправившему PUSH */
	uint32_t ret = 0;

	/* Проверяем пуст ли стек */
	if(st->is_not_empty)
	{
		/* Стек не пуст - формируем возвращаемое значение:
		 * 0b11<значение> */
		ret = 0xC0000000 | *(st->head);

		/* Удаляем элемент из верхушки стека */
		if(st->head == st->m)
			st->is_not_empty = 0;
		else
			st->head--;
	}

	union sigval sv;

	sv.sival_int = (int) ret;

	/* Сигналом SIGUSR2 возвращаем процессу, сделавшему POP (PID процесса в si->si_pid),
	 * код возврата (ret): 0 в случае ошибки, 0b11<значение> в случае успеха */
	if(
			sigqueue(si->si_pid, SIGUSR2, sv) == -1
	  )
		perror("Ошибка отправления ответа на pop");
}

/* Команда INIT (инициализация) */

void init()
{
	/* Выделяем память под стеки */
	if((stack = (struct s_stack*) malloc(sizeof(struct s_stack) * STACK_NUM)) == NULL)
	{
		perror("Ошибка выделения памяти под описатели стеков");
		exit(-1);
	}

	int8_t x;

	/* Для каждого стека */
	for(x = 0; x < STACK_NUM; x++)
	{
		/* Выделить память под стек */
		stack[x].m = stack[x].head = (uint32_t *) malloc(STACK_SIZE);

		if(stack[x].m == NULL)
		{
			perror("Ошибка выделения памяти под стеки");
			
			/* В случае ошибки выделения памяти - возвращаем всю память системе,
			 * что была выделена до ошибки - и завершаем менеджер стеков */
			for(x--; x > 0; x--)
				free(stack[x].m);
			free(stack);

			exit(-1);
		}

		/* Установить флаг, говорящий о пустоте стека */
		stack[x].is_not_empty = 0;
	}
}

/* Команда DEST (уничтожение стеков и завершение менеджера стеков) */

void destroy()
{
	uint8_t x;

	/* Возвращаем системе всю память */
	if(stack != NULL)
	{
		for(x = 0; x < STACK_NUM; x++)
			if(stack[x].m != NULL)
				free(stack[x].m);

		free(stack);
	}

	exit(0);
}

/* ############################################################################
 *							Родительский процесс							 */

/* Обработчик сигналов клиента - обработчик ответов */

void client_sig_proc(int signum, siginfo_t *si, void *notused)
{
	if(signum == SIGUSR1)
	{
		/* Сигналом SIGUSR1 пришел ответ на PUSH */

		if(si->si_int & 0xC0000000)
			/* Если у кода ответа старшие два бита равны единицам, то
			 * PUSH прошел успешно */
			printf("\tpush: успех\n");
		else
			/* Если код ответа равен 0, то PUSH прошел неудачно */
			printf("\tpush: ошибка - стек полон\n");
	}
	else	/* SIGUSR2 */
	{
		/* Сигналом SIGUSR2 пришел ответ на POP */

		if(si->si_int & 0xC0000000)
			/* Если у кода ответа старшие два бита равны единицам, то
			 * POP прошел успешно и в младших 30-и битах кода ответа -
			 * содержится интересующее нас значение */
			printf("\tpop: %u\n", si->si_int & 0x3FFFFFFF);
		else
			/* Если код ответа равен 0, то POP прошел неудачно */
			printf("\tpop: стек пуст\n");
	}
}

/* Макрос для PUSH
 *
 * (предполагается, что в вызывающей его функции есть переменная child
 * типа int, содержащая PID менеджера стеков */

#define PUSH(st_num, value) \
{\
	printf("push %u, %u\n", st_num, value);\
	operation(1, child, st_num, value);\
}

/* Макрос для POP
 *
 * (предполагается, что в вызывающей его функции есть переменная child
 * типа int, содержащая PID менеджера стеков */

#define POP(st_num) \
{\
	printf("pop %u\n", st_num);\
	operation(2, child, st_num, 0);\
}

/* Макрос для INIT
 *
 * (предполагается, что в вызывающей его функции есть переменная child
 * типа int, содержащая PID менеджера стеков */

#define INIT \
{\
	sleep(1);\
	printf("init\n");\
	kill(child, SIGHUP);\
	sleep(1);\
}

/* Макрос для DEST
 *
 * (предполагается, что в вызывающей его функции есть переменная child
 * типа int, содержащая PID менеджера стеков */

#define DEST \
{\
	printf("dest\n");\
	kill(child, SIGTERM);\
	waitpid(child, NULL, 0);\
}

#define PUSH_OP 1
#define POP_OP 2

/* Функция отправления PUSH (SIGUSR1) и POP (SIGUSR2)
 *
 * Параметры:
 *		op - код операции. PUSH_OP для PUSH, POP_OP для POP
 *		manager_pid - PID менеджера стеков
 *		st_num - номер стека (< STACK_NUM)
 *		value - значение (учитываются младшие 30 битов) */

void operation(const int op, const int manager_pid, const unsigned short st_num, const uint32_t value)
{
	/* Создаем пустой набор сигналов */
	sigset_t set;

	sigemptyset(& set);

	/* Формируем первое поле передаваемого с сигналами значения -
	 * номер целевого стека (старшие два бита данного значения) */
	union sigval sv;
	sv.sival_int = (st_num << 30);

	int sig;
	
	if(
			op == PUSH_OP
	  )
	{
		/* Передаем PUSH: сигнал SIGUSR1, в передаваемом значении второе поле
		 * (младшие 30 битов) - заталкиваемое в стек значение (value) */
		sig = SIGUSR1;
		sv.sival_int |= (value & 0x3FFFFFFF);
	}
	else
		/* Передаем POP: сигнал SIGUSR2 */
		sig = SIGUSR2;

	/* Отправляем сигнал */
	if(
			sigqueue(manager_pid, sig, sv) == -1
	  )
		perror("Ошибка при отправлении команды менеджеру стеков");
	else
		/* Временно меняем сигнальную маску процесса на пустую и ожидаем ответа */
		sigsuspend(& set);
}

/* ############################################################################ */

int main()
{
	printf("\n");

	/* Обоим процессам понадобится пустой набор сигналов */
	sigset_t set;

	sigemptyset(&set);

	/* Запускаем дочерний процесс - менеджер стеков */
	int child = fork();

	if(child == -1)
		perror("Ошибка при запуске менеджера стеков");
	else if(child)
	{
		/* Мы в родительском процессе - в клиенте */

		/* Клиент отправляет менеджеру стеков команду инициализации */
		INIT;

		/* Блокируем SIGUSR1, SIGUSR2 - ответы на push и pop необходимо
		 * принимать только после этих команд, а не в любое время */
		if(
				sigaddset(& set, SIGUSR1) == -1
				||
				sigaddset(& set, SIGUSR2) == -1
				||
				sigprocmask(SIG_SETMASK, & set, NULL) == -1
		  )
		{
			perror("Ошибка при добавлении сигналов в набор");

			/* Блокировка не удалась - это фатальная ошибка - завершаем оба процесса */

			DEST;

			return -1;
		}
		
		/* Описатель действия по получении сигнала */
		struct sigaction act;
		act.sa_sigaction = & client_sig_proc;	/* Адрес функции - обработчика */
		act.sa_mask = set;						/* Набор сигналов, блокируемых во время обработки сигнала */
		act.sa_flags = SA_SIGINFO;				/* Флаг SA_SIGINFO предписывает системе передавать в обработчик сигнала
												   дополнительную информацию о сигнале */

		/* На сигналы SIGUSR1 и SIGUSR2 устанавливаем обработчик */
		if(
				sigaction(SIGUSR1, &act, NULL) == -1
				||
				sigaction(SIGUSR2, &act, NULL) == -1
		  )
		{
			perror("Ошибка: клиент не может обрабатывать сигналы SIGUSR1, SIGUSR2");
			
			/* Установить обработчик на сигналы SIGUSR1 и SIGUSR2 не удалось - это фатальная ошибка - завершаем оба процесса */

			DEST;

			return -1;
		}

		/* Различные команды */
		PUSH(1, 1988);
		POP(1);
		POP(1);
		POP(0);
		PUSH(2, 5555);
		PUSH(3, 7777);
		PUSH(1, 1111);
		POP(2);
		POP(3);
		PUSH(1, 8);
		POP(1);
		POP(1);
		POP(1);

		/* Отправляем команду DEST (завершение) менеджеру стеков */
		DEST;
	}
	else
	{
		/* Мы в дочернем процессе - менеджере стеков */

		/* Блокируем сигналы, реализующие протокол -
		 * SIGUSR1, SIGUSR2, SIGTERM, SIGHUP */
		if(
				sigaddset(&set, SIGUSR1) == -1
				||
				sigaddset(&set, SIGUSR2) == -1
				||
				sigaddset(&set, SIGTERM) == -1
				||
				sigaddset(&set, SIGHUP) == -1
				||
				sigprocmask(SIG_SETMASK, &set, NULL) == -1
		  )
		{
			perror("Ошибка при блокировании сигналов менеджером стеков");
			exit(-1);
		}

		/* Описатель действия по получении сигнала */
		struct sigaction act;
		act.sa_sigaction = & manager_sig_proc;	/* Адрес функции - обработчика */
		act.sa_mask = set;						/* Набор сигналов, блокируемых во время обработки сигнала */
		act.sa_flags = SA_SIGINFO;				/* Флаг SA_SIGINFO предписывает системе передавать в обработчик сигнала
												   дополнительную информацию о сигнале */

		/* Для сигналов, реализующие протокол, устанавливаем
		 * обработчики */
		if(
				sigaction(SIGUSR1, & act, NULL) == -1
				||
				sigaction(SIGUSR2, & act, NULL) == -1
				||
				sigaction(SIGHUP, & act, NULL) == -1
				||
				sigaction(SIGTERM, & act, NULL) == -1
		  )
		{
			perror("Ошибка при установке обработчика сигналов SIGUSR1, SIGUSR2, SIGHUP, SIGTERM");
			exit(-1);
		}

		/* Очищаем набор */
		sigemptyset(& set);

		/* Главный цикл менеджера стеков */
		while(1)
			/* Временно установить сигнальную маску в (пустой) набор сигналов и
			 * ожидать сигнала (в частности, команду одним из сигналов: SIGUSR1, SIGUSR2,
			 * SIGHUP, SIGDEST) */
			sigsuspend(& set);
	}

	printf("\n");

	return 0;
}

