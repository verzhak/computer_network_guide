
/* 
Курсовой проект по дисциплине "Вычислительные сети"

Демонстрация выполнения задания лабораторной работы № 3

"Отправление пакетов типа ECHO протокола ICMP, получение пакетов типа ECHO-REPLY протокола ICMP"

Подготовил: Акинин М.В.

01.12.2010
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/ip.h>
#include <netinet/in.h>
#include <linux/icmp.h>

#define BUF_SIZE 4096

/* 
Функция подсчета контрольной суммы

Параметры:

	buf		 -	буфер, контрольную сумму содержимого которого необходимо подсчитать
	buf_size -	размер буфера в байтах

Возвращаемое значение:
	
	Контрольная сумма
*/
uint16_t checksum(uint16_t *buf, uint16_t buf_size);

/* Главная функция программы */
int main(const int argc, const char *argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "\nПрограмме передано недостаточное количество аргументов\n\n\
Формат вызова: ./program IP NUM\n\nЗдесь:\n\n\
\tIP\t-\tIPv4-адрес сетевого узла - получателя пакетов типа ECHO протокола ICMP;\n\
\tNUM\t-\tколичество отправляемых пакетов типа ECHO протокола ICMP.\n\n");
		return -1;
	}

	/* Создание сырого (SOCK_RAW) сокета для обмена по протоколу ICMP (IPPROTO_ICMP) */
	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(sock == -1)
	{
		fprintf(stderr, "\nОшибка при создании сокета\n\n");
		return -1;
	}

	unsigned u, max_u = atoi(argv[2]) + 1;
	ssize_t size;
	uint8_t echo_buf[BUF_SIZE], echo_reply_buf[BUF_SIZE];
	struct sockaddr_in addr;
	struct icmphdr *echo_hdr = (struct icmphdr *) echo_buf, *echo_reply_hdr = (struct icmphdr *) echo_reply_buf + sizeof(struct iphdr);

	/* Заполнение полей заголовка пакета типа ECHO протокола ICMP */
	echo_hdr->type = ICMP_ECHO;		// Тип отправляемого пакета - ECHO
	echo_hdr->code = 0;				// Для пакетов типа ECHO поле code не учитывается
	echo_hdr->un.echo.id = 53890;	// Идентификатор последовательности пакетов

	/* Заполнение описателя IP-адреса сетевого узла - получателя пакетов типа ECHO протокола ICMP */
	addr.sin_family = AF_INET;		// Семейство протоколов (стек протоколов TCP/IPv4)
	addr.sin_port = 0;				// Порт назначения (для протокола ICMP устанавливается 0, так как протокол ICMP не оперирует портами)
	inet_aton(argv[1], & addr.sin_addr);	// IP-адрес сетевого узла в сетевом порядке байт

	printf("\n");

	/* Главный цикл программы */
	for(u = 1; u < max_u; u++)
	{
		/* В поле заголовка ICMP-ECHO-пакета, хранящее номер пакета в последовательности, записывается номер отправляемого пакета */
		echo_hdr->un.echo.sequence = u << 8;

		/* Подсчет контрольной суммы ICMP-ECHO-пакета (контрольная сумма подсчитывается при поле контрольной суммы, установленном в 0) */
		echo_hdr->checksum = 0;
		echo_hdr->checksum = checksum((uint16_t *) echo_hdr, sizeof(struct icmphdr));

		/* Отправление ICMP-ECHO-пакета целевому сетевому узлу */
		sendto(sock, echo_buf, sizeof(struct icmphdr) + 100, 0, (struct sockaddr *) & addr, sizeof(struct sockaddr_in));

		printf("Пакет %u отправлен -> ", u);

		do
		{
			sleep(1);

			/* Получение ответного ICMP-ECHO_REPLY-пакета. Операция получения ответа не блокируется */
			size = recv(sock, echo_reply_buf, BUF_SIZE, MSG_DONTWAIT);
		}
		while (size < 0);

		if(size > 0)
			if(echo_reply_hdr->type == ICMP_ECHOREPLY)
				printf("ответ получен\n");
	}

	printf("\n");

	/* Уничтожение сокета */
	close(sock);

	return 0;
}

/* 
Функция подсчета контрольной суммы

Параметры:

	buf		 -	буфер, контрольную сумму содержимого которого необходимо подсчитать
	buf_size -	размер буфера в байтах

Возвращаемое значение:
	
	Контрольная сумма
*/
uint16_t checksum(uint16_t *buf, uint16_t buf_size)
{
	unsigned u;
	uint32_t sum = 0;
	buf_size /= 2;

	for(u = 0; u < buf_size; u++)
		sum += buf[u];

	return ~ ((sum & 0xFFFF) + (sum >> 16));
}

