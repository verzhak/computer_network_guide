
/* 
Курсовой проект по дисциплине "Вычислительные сети"

Демонстрация выполнения задания лабораторной работы № 1

"Получение таблицы маршрутизации чтением файла /proc/net/route"

Подготовил: Акинин М.В.

01.12.2010
*/

#include <stdio.h>

int main()
{
	/* Открытие файла /proc/net/route на чтение в режиме текстового файла */
	FILE *fl = fopen("/proc/net/route","r");
	if(fl == NULL)
	{
		fprintf(stderr, "\nОшибка при открытии файла /proc/net/route\n\n");
		return -1;
	}

	char iface[10], notused[15];
	unsigned long dest, gw, mask;

	/* Чтение заголовка таблицы */
	fscanf(fl, "%s %s %s %s %s %s %s %s %s %s %s\n", notused, notused, notused, notused, notused, notused, notused, notused, notused, notused, notused);

	/* Последовательное чтение записей о маршрутах */
	while(fscanf(fl, "%s %lX %lX %s %s %s %s %lX %s %s %s\n", iface, & dest, & gw, notused, notused, notused, notused, & mask, notused, notused, notused) != EOF)
	{
#define MES_GEN(mes, ip) printf("%s: %lu.%lu.%lu.%lu\n", mes, ip & 0xFF, (ip & 0xFF00) >> 8, (ip & 0xFF0000) >> 16, (ip & 0xFF000000) >> 24);

		/* Вывод записи об очередном маршруте */
		printf("\n#######################################################\n\n");
		printf("Сетевой интерфейс: %s\n", iface);
		MES_GEN("Сеть", dest);
		MES_GEN("Шлюз", gw);
		MES_GEN("Маска сети", mask);
		printf("\n#######################################################\n");
	}

	printf("\n");

	/* Закрытие файла */
	fclose(fl);

	return 0;
}

