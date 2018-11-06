
#include <stdio.h>

#include <sys/sysinfo.h>	/* Для sysinfo() */

int main()
{
	printf("\n");

	struct sysinfo si;

	/* Вызываем функцию sysinfo() для получения статистической информации о состоянии ОС */
	if(
		sysinfo(& si) == -1
	  )
		perror("Ошибка при получении статистической информации о состоянии ОС");
	else
	{
		printf("Статистическая информация о состоянии ОС:\n\n");
		
		printf("\tС момента загрузки ОС прошло: %lu секунд == %.3f минут == %.3f часов\n\n",
				si.uptime, si.uptime / 60.0, si.uptime / 3600.0);
		
		printf("\tЗагрузка (занятость) ОС за последнюю минуту: %.3f %%\n",
				si.loads[0] / 1000.0);
		printf("\tЗагрузка (занятость) ОС за последние 5 минуту: %.3f %%\n",
				si.loads[1] / 1000.0);
		printf("\tЗагрузка (занятость) ОС за последние 15 минуту: %.3f %%\n\n",
				si.loads[2] / 1000.0);

		printf("\tКоличество процессов, существующих в системе: %u\n\n", si.procs);

		printf("\tОбщий объем оперативной памяти: %lu байт == %.3f мегабайт\n",
				si.totalram * si.mem_unit, si.totalram * si.mem_unit / (1024.0 * 1024.0));
		printf("\tСвободно оперативной памяти: %lu байт == %.3f мегабайт\n",
				si.freeram * si.mem_unit, si.freeram * si.mem_unit / (1024.0 * 1024.0));
		printf("\tСуммарный объем разделяемых процессами областей оперативной памяти: %lu байт == %.3f мегабайт\n",
				si.sharedram * si.mem_unit, si.sharedram * si.mem_unit / (1024.0 * 1024.0));
		printf("\tСуммарный объем областей оперативной памяти, отведенных под страничный кэш: %lu байт == %.3f мегабайт\n\n",
				si.bufferram * si.mem_unit, si.bufferram * si.mem_unit / (1024.0 * 1024.0));

		printf("\tРазмер swap'а: %lu байт == %.3f мегабайт\n",
				si.totalswap * si.mem_unit, si.totalswap * si.mem_unit / (1024.0 * 1024.0));
		printf("\tСвободно swap'а: %lu байт == %.3f мегабайт\n\n",
				si.freeswap * si.mem_unit, si.freeswap * si.mem_unit / (1024.0 * 1024.0));

		printf("\tСуммарный объем областей оперативной памяти, отображаемых с помощью kmap: %lu байт == %.3f мегабайт\n",
				si.totalhigh * si.mem_unit, si.totalhigh * si.mem_unit / (1024.0 * 1024.0));
		printf("\tСуммарный объем свободных областей оперативной памяти, отображаемых с помощью kmap: %lu байт == %.3f мегабайт\n",
				si.freehigh * si.mem_unit, si.freehigh * si.mem_unit / (1024.0 * 1024.0));
	}

	printf("\n");

	return 0;
}

