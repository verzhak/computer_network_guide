
/* 
Курсовой проект по дисциплине "Вычислительные сети"

Демонстрация выполнения задания лабораторной работы № 3

"Отправление ARP-запроса на получение MAC-адреса для сетевого узла, IP-адрес которого указан оператором. Ожидание ARP-ответа"

Подготовил: Акинин М.В.

01.12.2010
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <net/if_arp.h>

#define BUF_SIZE 4096

/*
Определение идентификатора сетевого интерфейса по его имени

Параметры:

	sock - номер дескриптора пакетного сокета в таблице файловых дескрипторов процесса
	device_name - имя сетевого интерфейса

Возвращаемое значение:

	Идентификатор сетевого интерфейса - в случае успеха
	-1 - в случае неудачи
*/
int get_iface_id(int sock, const char* device_name);

/*
Побайтовое копирование в целевой буфер исходного MAC-адреса

Параметры:

	buf - указатель на целевой буфер
	mac - исходный MAC-адрес

Возвращаемое значение:

	0 - в случае успеха
	-1 - с случае неудачи
*/
int set_mac(uint8_t *buf, const char *mac);

/*
Побайтовое копирование в целевой буфер исходного IP-адреса

Параметры:

	buf - указатель на целевой буфер
	ip - исходный IP-адрес

Возвращаемое значение:

	0 - в случае успеха
	-1 - с случае неудачи
*/
int set_ip(uint8_t *buf, const char *ip);

/*
Конвертирование исходного MAC-адреса в его строковое представление в нотации ЧИСЛО:ЧИСЛО:ЧИСЛО:ЧИСЛО:ЧИСЛО:ЧИСЛО

Параметры:

	mac - исходный MAC-адрес

Возвращаемое значение:

	Строковое представление исходного MAC-адреса в указанной нотации - в случае успеха
	NULL - в случае неудачи
*/
char *mac_to_str(const uint8_t *mac);

/* Главная функция программы */
int main(const int argc, const char *argv[])
{
	if(argc != 5)
	{
		fprintf(stderr, "\nПрограмме передано недостаточное количество аргументов\n\nФормат вызова: ./program IFACE IMAC IIP OIP\n\nЗдесь:\n\n\
\tIFACE\t-\tимя сетевого интерфейса сетевого адаптера сетевого узла - отправителя;\n\
\tIMAC\t-\tMAC-адрес сетевого адаптера сетевого узла - отправителя;\n\
\tIIP\t-\tIPv4-адрес сетевого узла - отправителя;\n\
\tOIP\t-\tIPv4-адрес, соответствующий MAC-адрес для которого необходимо определить.\n\n");
		return -1;
	}

	/* Создание пакетного (AF_PACKET) сокета для обмена по протоколу ARP (ETH_P_ARP) без манипулирования заголовками Ethernet-кадров (SOCK_DGRAM) */
	int sock = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ARP));
	if(sock == -1)
	{
		fprintf(stderr, "\nОшибка: создать сокет не удалось\n\n");
		return -1;
	}

	uint8_t buf[BUF_SIZE];
	struct arphdr *hdr = (struct arphdr *) buf;
	size_t aoff = sizeof(struct arphdr);

	/* Формирование ARP-запроса */
	hdr->ar_hrd = htons(1); // Идентификатор протокола канального уровня (Ethernet)
	hdr->ar_pro = htons(0x0800); // Идентификатор протокола сетевого уровня (IPv4)
	hdr->ar_hln = 6; // Длина в байтах адреса протокола канального уровня (MAC-адрес - 6 байт)
	hdr->ar_pln = 4; // Длина в байтах адреса протокола сетевого уровня (IPv4-адрес- 4 байта)
	hdr->ar_op	= htons(1); // Код операции - 1 - ARP-запрос

	/* В буфер помещается MAC-адрес сетевого адаптера сетевого узла - отправителя */
	if(set_mac(buf + aoff, argv[2]))
	{
		fprintf(stderr, "\nОшибка при копировании MAC-адреса сетевого адаптера сетевого узла - отправителя в буфер\n\n");
		close(sock);
		return -1;
	}

	/* В буфер помещается IP-адрес сетевого узла - отправителя */
	if(set_ip(buf + aoff + 6, argv[3]))
	{
		fprintf(stderr, "\nОшибка при копировании IP-адреса сетевого узла - отправителя в буфер\n\n");
		close(sock);
		return -1;
	}

	/* Поле MAC-адреса сетевого узла - получателя устанавливается в 0 */
	buf[aoff + 10] = 0;
	buf[aoff + 11] = 0;
	buf[aoff + 12] = 0;
	buf[aoff + 13] = 0;
	buf[aoff + 14] = 0;
	buf[aoff + 15] = 0;

	/* В буфер помещается IP-адрес сетевого узла - получателя */
	if(set_ip(buf + aoff + 16, argv[4]))
	{
		fprintf(stderr, "\nОшибка при копировании IP-адреса сетевого узла - получателя в буфер\n\n");
		close(sock);
		return -1;
	}

	struct sockaddr_ll addr;

	memset(& addr, 0, sizeof(struct sockaddr_ll));

	/* Заполнение описателя адреса протокола канального уровня (в данном случае, описателя MAC-адреса) */
	addr.sll_family = AF_PACKET; // Используемое сокетом семейство протоколов
	addr.sll_protocol = htons(ETH_P_ARP); // Используемый сокетом протокол
	addr.sll_hatype = 1; // Код операции (ARP-запрос)
	addr.sll_pkttype = PACKET_BROADCAST; // Тип адреса (широковещательный)
	addr.sll_halen = 6; // Длина адреса в байтах
	memset(addr.sll_addr, 0xFF, 8); // Запись адреса FF:FF:FF:FF:FF:FF в поле sll_addr (два лишних байта не будут использованы)

	/* Получение идентификатора сетевого интерфейса */
	if(
		(addr.sll_ifindex = get_iface_id(sock, argv[1])) == -1
	  )
	{
		fprintf(stderr, "\nОшибка при получении идентификатора сетевого интерфейса\n\n");
		close(sock);
		return -1;
	}

	/* Отправление ARP-запроса */
	ssize_t size = sendto(sock, buf, aoff + 20, 0, (struct sockaddr *) & addr, sizeof(struct sockaddr_ll));

	if(size > 0)
	{
		char *temp;

		do
			/* Получение ARP-ответа */
			size = recv(sock, buf, BUF_SIZE, 0);
		while(ntohs(hdr->ar_op) != 2);

		if(
			(size > 0)
			&&
			(temp = mac_to_str(buf + aoff + 10)) != NULL
		  )
		{
			/* Вывод искомого соответствия IP-адреса сетевого узла - получателя MAC-адресу сетевого адаптера сетевого узла - получателя */
			printf("\nПолучен ARP-ответ: %s -> %s\n\n", argv[4], temp);
			free(temp);
		}
		else
		{
			fprintf(stderr, "\nОшибка при получении ARP-ответа\n\n");
			close(sock);
			return -1;
		}

	}
	else
	{
		fprintf(stderr, "\nОшибка при отправлении ARP-запроса\n\n");
		close(sock);
		return -1;
	}

	/* Уничтожение сокета */
	close(sock);

	return 0;
}

/*
Определение идентификатора сетевого интерфейса по его имени

Параметры:

	sock - номер дескриптора пакетного сокета в таблице файловых дескрипторов процесса
	device_name - имя сетевого интерфейса

Возвращаемое значение:

	Идентификатор сетевого интерфейса - в случае успеха
	-1 - в случае неудачи
*/
int get_iface_id(int sock, const char* device_name)
{
	struct ifreq req;
	strcpy(req.ifr_name, device_name);

	/* Получение идентификатора сетевого интерфейса с помощью системного вызова ioctl и действия SIOCGIFINDEX */
	if(ioctl(sock, SIOCGIFINDEX, & req))
		return -1;
	
	return req.ifr_ifindex;
}

/*
Побайтовое копирование в целевой буфер исходного MAC-адреса

Параметры:

	buf - указатель на целевой буфер
	mac - исходный MAC-адрес

Возвращаемое значение:

	0 - в случае успеха
	-1 - с случае неудачи
*/
int set_mac(uint8_t *buf, const char *mac)
{
	if(strlen(mac) != 17)
		return -1;

	unsigned u, v;
	char temp[18];

	strcpy(temp, mac);

	for(u = 0, v = 0; u < 17; u += 3, v ++)
	{
		temp[u + 2] = '\0';
		buf[v] = strtoul(temp + u, NULL, 16);
	}

	return 0;
}

/*
Побайтовое копирование в целевой буфер исходного IP-адреса

Параметры:

	buf - указатель на целевой буфер
	ip - исходный IP-адрес

Возвращаемое значение:

	0 - в случае успеха
	-1 - с случае неудачи
*/
int set_ip(uint8_t *buf, const char *ip)
{
	/* Можно было бы использовать функцию inet_network(), но она не описана в курсе лабораторных работ */

	int v;
	unsigned mask = 0xFF000000;

	in_addr_t addr = inet_addr(ip);

	for(v = 3; v >= 0; v--, mask >>= 8)
		buf[v] = (addr & mask) >> (v << 3);

	return 0;
}

/*
Конвертирование исходного MAC-адреса в его строковое представление в нотации ЧИСЛО:ЧИСЛО:ЧИСЛО:ЧИСЛО:ЧИСЛО:ЧИСЛО

Параметры:

	mac - исходный MAC-адрес

Возвращаемое значение:

	Строковое представление исходного MAC-адреса в указанной нотации - в случае успеха
	NULL - в случае неудачи
*/
char *mac_to_str(const uint8_t *mac)
{
	char *temp = malloc(18  * sizeof(char));
	if(temp == NULL)
		return NULL;

	sprintf(temp, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	return temp;
}

