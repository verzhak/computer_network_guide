
/* 
Курсовой проект по дисциплине "Вычислительные сети"

Демонстрация выполнения задания лабораторной работы № 2

Подготовил: Акинин М.В.

01.12.2010
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUF_SIZE 131072 // 128 килобайт

/*
Функция вывода в стандартный поток ошибок сообщения об ошибке

Параметры:

	mes - сообщение об ошибке

Возвращаемое значение:

	-1
*/
int error(const char *mes);

/*
Вывод в стандартный поток вывода сообщения о получении очередной команды серверной компонентой

Параметры:

	com - полученная команда
	is_known - единица в случае, если команда корректна, ноль в случае, если команда некорректна
*/
void get_command(const char *com, char is_known);

/*
Главная функция серверной компоненты, использующей протокол UDP для обмена данными с управляющей компонентой

Параметры:

	s_port - номер UDP-порта, прослушиваемого серверной компонентой

Возвращаемое значение:

	0 - в случае успешного завершения компоненты
	-1 - в случае неудачного завершения компоненты
*/
int server_udp_main(const char *s_port);

/*
Главная функция серверной компоненты, использующей протокол TCP для обмена данными с управляющей компонентой

Параметры:

	s_port - номер TCP-порта, прослушиваемого серверной компонентой

Возвращаемое значение:

	0 - в случае успешного завершения компоненты
	-1 - в случае неудачного завершения компоненты
*/
int server_tcp_main(const char *s_port);

/*
Главная функция управляющей компоненты

Параметры:

	s_server_udp_addr - IP-адрес серверной компоненты, доступной по протоколу UDP
	s_server_udp_port - UDP-порт, на котором запущена серверная компонента, доступная по протоколу UDP
	s_server_tcp_addr - IP-адрес серверной компоненты, доступной по протоколу TCP
	s_server_tcp_port - TCP-порт, на котором запущена серверная компонента, доступная по протоколу TCP

Возвращаемое значение:

	0 - в случае успешного завершения компоненты
	-1 - в случае неудачного завершения компоненты
*/
int control_main(const char *s_server_udp_addr, const char *s_server_udp_port, const char *s_server_tcp_addr, const char *s_server_tcp_port);

/* Главная функция программы */
int main(const int argc, const char *argv[])
{
#define HELP return error("Недостаточное количество аргументов\n\nФормат запуска:\n\n\
\t./program udp PORT\t-\tдля запуска серверной компоненты, осуществляющей обмен с управляющей компонентой по UDP порту с номером PORT\n\
\t./program tcp PORT\t-\tдля запуска серверной компоненты, осуществляющей обмен с управляющей компонентой по TCP порту с номером PORT\n\
\t./program control UDP_ADDR UDP_PORT TCP_ADDR TCP_PORT\t-\tдля запуска управляющей компоненты, подключающейся к серверными компонентами с \
IP-адресами UDP_ADDR и TCP_ADDR к портам UDP_PORT и TCP_PORT соответственно\n");

	if(argc < 3)
		HELP;

	/* Запуск одной из серверных компонент или управляющей компоненты в зависимости от переданного программе первого, не считая имени исполняемого файла, аргумента */
	if(
		(! strcmp(argv[1], "udp"))
		&&
		(argc == 3)
	  )
		return server_udp_main(argv[2]);
	else if(
			(! strcmp(argv[1], "tcp"))
			&&
			(argc == 3)
		   )
		return server_tcp_main(argv[2]);
	else if(
			(! strcmp(argv[1], "control"))
			&&
			(argc == 6)
		   )
		return control_main(argv[2], argv[3], argv[4], argv[5]);
	else
		HELP;

	return 0;
}

/*
Функция вывода в стандартный поток ошибок сообщения об ошибке

Параметры:

	mes - сообщение об ошибке

Возвращаемое значение:

	-1
*/
int error(const char *mes)
{
	fprintf(stderr, "[Ошибка]: %s\n", mes);
	fflush(stderr);

	return -1;
}

/*
Вывод в стандартный поток вывода сообщения о получении очередной команды серверной компонентой

Параметры:

	com - полученная команда
	is_known - единица в случае, если команда корректна, ноль в случае, если команда некорректна
*/
void get_command(const char *com, char is_known)
{
	if(is_known)
		printf("[Команда]: %s\n", com);
	else
		fprintf(stderr, "[Ошибка]: Неизвестная команда \"%s\"\n", com);
}

/*
Получение содержимого таблицы маршрутизации сетевого узла

Параметры:

	buf - указатель на буфер, в который будет сохранено текстовое представление таблицы маршрутизации

Возвращаемое значение:

	0 - в случае успеха
	-1 - в случае неудачи
*/
int get_route(char *buf)
{
	/* Открытие файла /proc/net/route на чтение в режиме текстового файла */
	FILE *fl = fopen("/proc/net/route","r");
	if(fl == NULL)
		return -1;

	char temp[1024], iface[10], notused[15];
	unsigned long dest, gw, mask;

	buf[0] = '\0';

	/* Чтение заголовка таблицы */
	fscanf(fl, "%s %s %s %s %s %s %s %s %s %s %s\n", notused, notused, notused, notused, notused, notused, notused, notused, notused, notused, notused);

	/* Последовательное чтение записей о маршрутах */
	while(fscanf(fl, "%s %lX %lX %s %s %s %s %lX %s %s %s\n", iface, & dest, & gw, notused, notused, notused, notused, & mask, notused, notused, notused) != EOF)
	{
#define MES_GEN(mes, ip) sprintf(temp, "%s: %lu.%lu.%lu.%lu\n", mes, ip & 0xFF, (ip & 0xFF00) >> 8, (ip & 0xFF0000) >> 16, (ip & 0xFF000000) >> 24);\
		strcat(buf, temp)

		/* Вывод записи об очередном маршруте */
		strcat(buf, "\n#######################################################\n\n");
		sprintf(temp, "Сетевой интерфейс: %s\n", iface);
		strcat(buf, temp);
		MES_GEN("Сеть", dest);
		MES_GEN("Шлюз", gw);
		MES_GEN("Маска сети", mask);
		strcat(buf, "\n#######################################################\n");
	}

	/* Закрытие файла */
	fclose(fl);

	return 0;
}

/*
Главная функция серверной компоненты, использующей протокол UDP для обмена данными с управляющей компонентой

Параметры:

	s_port - номер UDP-порта, прослушиваемого серверной компонентой

Возвращаемое значение:

	0 - в случае успешного завершения компоненты
	-1 - в случае неудачного завершения компоненты
*/
int server_udp_main(const char *s_port)
{
	/* Создание UDP-сокета (0 означает протокол по умолчанию для комбинации значений AF_INET и SOCK_DGRAM - таковым протоколом является протокол UDP) */
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
		return error("Невозможно создать сокет");

	struct sockaddr_in addr, cli_addr;
	socklen_t notused;

	/* Заполнение описателя IP-адреса и номера UDP-порта, на которых будет запущена серверная компонента */
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(s_port));
	inet_aton("0.0.0.0", & addr.sin_addr);

	/* Привязка сокета к IP-адресу и UDP-порту */
	if(bind(sock, (struct sockaddr *) & addr, sizeof(struct sockaddr_in)))
	{
		close(sock);
		return error("Неудачное привязывание сокета к порту");
	}

	char buf[BUF_SIZE + 1];
	ssize_t size;

	/* Главный цикл серверной компоненты */
	while(1)
	{
		/* Получение очередной команды от управляющей компоненты */
		size = recvfrom(sock, buf, BUF_SIZE, 0, (struct sockaddr *) & cli_addr, & notused);

		if(size > 0)
		{
			if(buf[size - 1] == 0xA)
				buf[size - 1] = '\0';
			else
				buf[size] = '\0';

			/* Обработка команды и отправление ответа */
			if(! strcmp(buf, "halt"))
			{
				get_command("halt", 1);

				close(sock);

				return 0;
			}
			else if(! strcmp(buf, "ping"))
			{
				get_command("ping", 1);

				strcpy(buf, "pong");
				sendto(sock, buf, strlen(buf) + 1, 0, (struct sockaddr *) & cli_addr, sizeof(struct sockaddr_in));
			}
			else if(! strcmp(buf, "rand"))
			{
				get_command("rand", 1);

				sprintf(buf, "%d", rand());
				sendto(sock, buf, strlen(buf) + 1, 0, (struct sockaddr *) & cli_addr, sizeof(struct sockaddr_in));
			}
			else if(! strcmp(buf, "route"))
			{
				get_command("route", 1);

				if(get_route(buf))
				{
					error("Ошибка при открытии файла /proc/net/route");
					buf[0] = '\0';
				}

				sendto(sock, buf, strlen(buf) + 1, 0, (struct sockaddr *) & cli_addr, sizeof(struct sockaddr_in));
			}
			else
				get_command(buf, 0);
		}
	}
}

/*
Главная функция серверной компоненты, использующей протокол TCP для обмена данными с управляющей компонентой

Параметры:

	s_port - номер TCP-порта, прослушиваемого серверной компонентой

Возвращаемое значение:

	0 - в случае успешного завершения компоненты
	-1 - в случае неудачного завершения компоненты
*/
int server_tcp_main(const char *s_port)
{
	/* Создание TCP-сокета (0 означает протокол по умолчанию для комбинации значений AF_INET и SOCK_STREAM - таковым протоколом является протокол TCP) */
	int cli_sock, sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1)
		return error("Невозможно создать сокет");

	struct sockaddr_in addr;

	/* Заполнение описателя IP-адреса и номера TCP-порта, на которых будет запущена серверная компонента */
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(s_port));
	inet_aton("0.0.0.0", & addr.sin_addr);

	/* Привязка сокета к IP-адресу и TCP-порту */
	if(bind(sock, (struct sockaddr *) & addr, sizeof(struct sockaddr_in)))
	{
		close(sock);
		return error("Неудачное привязывание сокета к порту");
	}

	/* Перевод сокета в слушающий режим */
	if(listen(sock, 5))
	{
		close(sock);
		return error("Неудачный перевод сокета в режим прослушивания");
	}

	/* Ожидание подключения управляющей компоненты */
	if(
		(cli_sock = accept(sock, NULL, NULL)) == -1
	  )
	{
		close(sock);
		return error("Неудачное подключение клиента");
	}

	char buf[BUF_SIZE + 1];
	ssize_t size;

	/* Главный цикл серверной компоненты */
	while(1)
	{
		/* Получение очередной команды от управляющей компоненты */
		size = recv(cli_sock, buf, BUF_SIZE, 0);

		if(size > 0)
		{
			if(buf[size - 1] == 0xA)
				buf[size - 1] = '\0';
			else
				buf[size] = '\0';

			/* Обработка команды и отправление ответа */
			if(! strcmp(buf, "halt"))
			{
				get_command("halt", 1);

				close(cli_sock);
				close(sock);

				return 0;
			}
			else if(! strcmp(buf, "ping"))
			{
				get_command("ping", 1);

				strcpy(buf, "pong");
				send(cli_sock, buf, strlen(buf) + 1, 0);
			}
			else if(! strcmp(buf, "rand"))
			{
				get_command("rand", 1);

				sprintf(buf, "%d", rand());
				send(cli_sock, buf, strlen(buf) + 1, 0);
			}
			else if(! strcmp(buf, "route"))
			{
				get_command("route", 1);

				if(get_route(buf))
				{
					error("Ошибка при открытии файла /proc/net/route");
					buf[0] = '\0';
				}

				send(cli_sock, buf, strlen(buf) + 1, 0);
			}
			else
				get_command(buf, 0);
		}
	}
}

/*
Главная функция управляющей компоненты

Параметры:

	s_server_udp_addr - IP-адрес серверной компоненты, доступной по протоколу UDP
	s_server_udp_port - UDP-порт, на котором запущена серверная компонента, доступная по протоколу UDP
	s_server_tcp_addr - IP-адрес серверной компоненты, доступной по протоколу TCP
	s_server_tcp_port - TCP-порт, на котором запущена серверная компонента, доступная по протоколу TCP

Возвращаемое значение:

	0 - в случае успешного завершения компоненты
	-1 - в случае неудачного завершения компоненты
*/
int control_main(const char *s_server_udp_addr, const char *s_server_udp_port, const char *s_server_tcp_addr, const char *s_server_tcp_port)
{
	int sock[2];
	
	/* Создание UDP-сокета (0 означает протокол по умолчанию для комбинации значений AF_INET и SOCK_DGRAM - таковым протоколом является протокол UDP) */
	sock[0] = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock[0] == -1)
		return error("Невозможно создать UDP-сокет");

	if(
		/* Создание TCP-сокета (0 означает протокол по умолчанию для комбинации значений AF_INET и SOCK_STREAM - таковым протоколом является протокол TCP) */
		(sock[1] = socket(AF_INET, SOCK_STREAM, 0)) == -1
	  )
	{
		close(sock[0]);
		return error("Невозможно создать TCP-сокет");
	}

	struct sockaddr_in addr[2];

	/* Заполнение описателя IP-адреса и номера UDP-порта соответствующей серверной компоненты */
	addr[0].sin_family = AF_INET;
	addr[0].sin_port = htons(atoi(s_server_udp_port));
	inet_aton(s_server_udp_addr, & addr[0].sin_addr);

	/* Заполнение описателя IP-адреса и номера TCP-порта соответствующей серверной компоненты */
	addr[1].sin_family = AF_INET;
	addr[1].sin_port = htons(atoi(s_server_tcp_port));
	inet_aton(s_server_tcp_addr, & addr[1].sin_addr);

	/* Подключение TCP-сокета к TCP-сокету соответствующей серверной компоненты */
	if(connect(sock[1], (struct sockaddr *) & addr[1], sizeof(struct sockaddr_in)))
	{
		close(sock[0]);
		close(sock[1]);
		return error("Ошибка при подключении к серверной компоненте, использующей протокол TCP для обмена с управляющей компонентой");
	}

	int node, num[2];
	unsigned com_sc = 0;
	char buf[BUF_SIZE + 1], com[50];
	ssize_t size;

	/* Главный цикл управляющей компоненты */
	while(1)
	{
		com_sc++;

		printf(" >>> ");
		scanf("%s", com);

		/* Обработка команд оператора */
		if(! strcmp(com, "halt"))
		{
			sendto(sock[0], com, strlen(com), 0, (struct sockaddr *) & addr[0], sizeof(struct sockaddr_in));
			send(sock[1], com, strlen(com), 0);

			close(sock[0]);
			close(sock[1]);

			return 0;
		}
		else if(! strcmp(com, "ping"))
		{
			scanf("%d", & node);

			if((node == 1) || (node == 2))
			{
				if(node == 1)
				{
					sendto(sock[0], com, strlen(com), 0, (struct sockaddr *) & addr[0], sizeof(struct sockaddr_in));
					size = recv(sock[0], buf, BUF_SIZE, 0);
				}
				else
				{
					send(sock[1], com, strlen(com), 0);
					size = recv(sock[1], buf, BUF_SIZE, 0);
				}

				if(size > 0)
				{
					buf[size] = '\0';

					if(! strcmp(buf, "pong"))
						printf("pong от узла %d\n", node);
				}
			}
			else
			{
				com_sc--;
				error("Неизвестный номер сетевого узла");
			}
		}
		else if(! strcmp(com, "rand"))
		{
			sendto(sock[0], com, strlen(com), 0, (struct sockaddr *) & addr[0], sizeof(struct sockaddr_in));
			send(sock[1], com, strlen(com), 0);

			size = recv(sock[0], buf, BUF_SIZE, 0);
			buf[size] = '\0';
			num[0] = atoi(buf);

			size = recv(sock[1], buf, BUF_SIZE, 0);
			buf[size] = '\0';
			num[1] = atoi(buf);

			printf("0x%X\n", num[0] & num[1]);
		}
		else if(! strcmp(com, "route"))
		{
			scanf("%d", & node);

			if((node == 1) || (node == 2))
			{
				if(node == 1)
				{
					sendto(sock[0], com, strlen(com), 0, (struct sockaddr *) & addr[0], sizeof(struct sockaddr_in));
					size = recv(sock[0], buf, BUF_SIZE, 0);
				}
				else
				{
					send(sock[1], com, strlen(com), 0);
					size = recv(sock[1], buf, BUF_SIZE, 0);
				}

				if(size > 0)
				{
					buf[size] = '\0';
					printf("Таблица маршрутизации сетевого узла %d:\n\n%s\n", node, buf);
				}
			}
			else
			{
				com_sc--;
				error("Неизвестный номер сетевого узла");
			}
		}
		else if(! strcmp(com, "stat"))
			printf("Всего выполнено %u корректных команд\n", com_sc);
		else
		{
			com_sc--;
			get_command(com, 0);
		}
	}
}

