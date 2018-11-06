
#include <linux/module.h>	/* Базовые заголовочные файлы, необходимые модулю */
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/proc_fs.h>	/* Для функционала, связанного с каталогом /proc */
#include <linux/uaccess.h>	/* Для copy_from_user() */
#include <linux/sched.h>	/* Структура данных task_struct и функция find_task_by_vpid() */

#define PID_BUF_LEN 7

/* Описатель содержимого создаваемого нами файла /proc/kurs/pid */
struct s_pid_buf
{
	char buf[PID_BUF_LEN];	/* Буфер */
	pid_t pid;				/* PID */
} pid_data;

/* Функция pid_read() будет вызвана, когда пользователь попытается прочитать файл /proc/kurs/pid */
static int pid_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

	/* Параметр data содержит указатель на описатель содержимого читаемого файла */
	struct s_pid_buf *pd = (struct s_pid_buf *) data;

	/* Записываем в буфер page, содержимое которого будет возвращено пользователю, содержимое буфера
	 * описателя */
	len = sprintf(page, pd->buf);

	/* Возвращаем ядру количество записанных в page байт */
	return len;
}

/* Функция pid_write() будет вызвана, когда пользователь запишет некоторые данные в файл /proc/kurs/pid */
static int pid_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int len;
	char *temp;

	/* Параметр data содержит указатель на описатель содержимого обрабатываемого файла */
	struct s_pid_buf *pd = (struct s_pid_buf *) data;
	
	/* Параметр count - количество записанных байт
	 *
	 * Анализируем, поместятся ли все записанные байты в буфер описателя содержимого файла */
	if(count < PID_BUF_LEN - 2)
		len = count;
	else
		len = PID_BUF_LEN - 2;

	/* Помещаем в буфер описателя данные, записанные пользователем в файл
	 *
	 * Функция copy_from_user() копирует len байт данных из буфера buffer, расположенного в пользовательской части виртуального адресного пространства
	 * некоторого процесса, в буфер описателя, расположенного в пространстве ядра */
	if(copy_from_user(pd->buf, buffer, len))
		return -EFAULT;

	if(pd->buf[len - 1] != '\n')
	{
		pd->buf[len] = '\n';
		len++;
	}

	pd->buf[len] = '\0';

	/* С помощью функции simple_strtol() (описанной в <linux/kernel.h>) анализируем
	 * буфер описателя (фактически, строку) и получаем записанное в него целое - PID целевого процесса */
	temp = NULL;
	pd->pid = simple_strtol(pd->buf, & temp, 10);

	if((temp != NULL && *temp != '\n') || pd->pid <= 0)
	{
		printk("------- [kurs] Invalid PID\n");

		/* Если буфер описателя содержит некорректный PID - обнуляем значение полей описателя */
		strcpy(pd->buf, "0\n");
		pd->pid = 0;
	}
	else
	{
		/* Если модулю передан корректный PID - получаем с помощью функции find_task_by_vpid() указатель
		 * на дескриптор целевого процесса по его PID'у */
		struct task_struct *ts = find_task_by_vpid(pd->pid);

		if(ts == NULL)
			/* Если find_task_by_vpid() вернула нулевой указатель, то процесс с указанным PID'ом не существует */
			printk("------- [kurs] Process with PID = %d not found\n", pd->pid);
		else
		{
			/* Указатель на дескриптор процесса получен - выведем информацию о процессе */

			printk("#############################################\n------- [kurs] Begin process information\n\n");

			/* PID и PGID процесса */
			printk("\t\tPID = %d\n\t\tProcess group ID (PGID) = %d\n\n", ts->pid, ts->tgid);

			/* Состояние процесса */
			printk("\t\tState: ");
			switch(ts->state)
			{
				case TASK_RUNNING:
					{
						printk("TASK_RUNNING");
						break;
					}
				case TASK_INTERRUPTIBLE:
					{
						printk("TASK_INTERRUPTIBLE");
						break;
					}
				case TASK_UNINTERRUPTIBLE:
					{
						printk("TASK_UNINTERRUPTIBLE");
						break;
					}
				case TASK_WAKEKILL:
					{
						printk("TASK_WAKEKILL");
						break;
					}
				case TASK_KILLABLE:
					{
						printk("TASK_KILLABLE");
						break;
					}
				case TASK_STOPPED:
					{
						printk("TASK_STOPPED");
						break;
					}
				case TASK_TRACED:
					{
						printk("TASK_TRACED");
						break;
					}
				default:
					printk("Unknown");
			}
			printk("\n\n");

			/* Представления приоритета на уровне ядра */
			printk("\t\tPriority:\n\t\t\tPriority = %d\n\t\t\tStatic priority = %d\n\t\t\tNormal priority = %d\n\t\t\tReal-time priority = %u\n\n",
					ts->prio, ts->static_prio, ts->normal_prio, ts->rt_priority);

			/* Политика планирования и рассчитанные любезность и статический приоритет (в том виде, в котором он используется
			 * процессами пользовательского пространства) */
			printk("\t\tScheduling policy: ");
			switch(ts->policy)
			{
				case SCHED_NORMAL:	/* Константы компилятора SCHED_NORMAL и SCHED_OTHER тождественны */
					{
						printk("SCHED_OTHER\n\t\t\tNice = %d", DEFAULT_PRIO - ts->prio);
						break;
					}
				case SCHED_IDLE:
					{
						printk("SCHED_IDLE");
						break;
					}
				case SCHED_BATCH:
					{
						printk("SCHED_BATCH\n\t\t\tNice = %d", DEFAULT_PRIO - ts->prio);
						break;
					}
				case SCHED_RR:
					{
						printk("SCHED_RR\n\t\t\tStatic priority (in userspace meaning) = %d", MAX_RT_PRIO - ts->rt_priority);
						break;
					}
				case SCHED_FIFO:
					{
						printk("SCHED_FIFO\n\t\t\tStatic priority (in userspace meaning) = %d", MAX_RT_PRIO - ts->rt_priority);
						break;
					}
				default:
					printk("Other");
			}
			printk("\n\n");
			
			/* PID процесса, принимающего от данного сигнал SIGCHLD по завершении, и PID настоящего родителя
			 * (часто оба PID'а совпадают) */
			printk("\t\tCurrent \"parent\" PID (this process get SIGCHLD signal) = %d\n\t\tReal parent PID = %d\n\n",
					ts->parent->pid, ts->real_parent->pid);

			/* UID'ы и GID'ы процесса */
			printk("\t\tUID:\n\t\t\tReal = %d\n\t\t\tEffective = %d\n\t\t\tSaved = %d\n\t\t\tFile system = %d\n",
					ts->uid, ts->euid, ts->suid, ts->fsuid);
			printk("\t\tGID:\n\t\t\tReal = %d\n\t\t\tEffective = %d\n\t\t\tSaved = %d\n\t\t\tFile system = %d\n\n",
					ts->gid, ts->egid, ts->sgid, ts->fsgid);

			/* Командная строка запуска процесса, если существует */
			if(ts->comm != NULL)
				printk("\t\tCommand = \"%s\"\n\n", ts->comm);

			printk("------- [kurs] End process information\n#############################################\n");
		}
	}

	return len;
}

/* Два указателя на описатели объектов каталога /proc */
static struct proc_dir_entry *kurs_dir, *pid_file;

/* Функция инициализации модуля */
static int __init kurs_init(void)
{
	/* В каталоге /proc создаем подкаталог /proc/kurs */
	kurs_dir = proc_mkdir("kurs", NULL);

	/* В каталоге /proc/kurs создаем файл /proc/kurs/pid с возможностью (существующей у любого пользователя системы)
	 * чтения из него и записи в него */
	pid_file = create_proc_entry("pid", 0666, kurs_dir);

	/* Инициализация полей описателя содержимого файла */
	strcpy(pid_data.buf, "0\n");
	pid_data.pid = 0;

	/* Заполняем поля описателя файла /proc/kurs/pid */
	pid_file->data = & pid_data;		/* Указатель на описатель содержимого файла */
	pid_file->read_proc = pid_read;		/* Указатель на функцию, которая будет обрабатывать запросы на чтение файла */
	pid_file->write_proc = pid_write;	/* Указатель на функцию, которая будет обрабатывать запросы на запись в файл */

	/* Для каталога /proc/kurs и файла /proc/kurs/pid устанавливаем поле, содержащее указание на модуль - владелец объектов
	 * каталога /proc - это данный модуль */
	kurs_dir->owner = THIS_MODULE;
	pid_file->owner = THIS_MODULE;

	printk("------- [kurs] Module load\n");

	return 0;
}

/* Функция - деструктор модуля */
static void __exit kurs_exit(void)
{
	/* Удаляем файл /proc/kurs/pid */
	remove_proc_entry("pid", kurs_dir);

	/* Удаляем каталог /proc/kurs */
	remove_proc_entry("kurs", NULL);

	printk("------- [kurs] Module unload\n");
}

/* Указываем ядру, что функция kurs_init() суть есть функция инициализации модуля */
module_init(kurs_init);
/* Указываем ядру, что функция kurs_exit() суть есть функция - деструктор модуля */
module_exit(kurs_exit);

MODULE_AUTHOR("Akinin M.V.");		/* Имя автора модуля */
MODULE_DESCRIPTION("Kurs Module");	/* Описание модуля */
MODULE_VERSION("1.0");				/* Версия модуля */
MODULE_LICENSE("GPL");				/* Лицензия модуля (предполагается GPLv2) */

