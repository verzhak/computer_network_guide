
/* 
Курсовой проект по дисциплине "Вычислительные сети"

Демонстрация выполнения задания лабораторной работы № 4

"Перехват пакетов протокола TCP (дополнительное задание повышенной сложности)"

Подготовил: Акинин М.В.

01.12.2010
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/if.h>
#include <linux/if_ether.h>

/* Размер вспомогательного буфера, в который будет считываться содержимое очередного перехваченного Ethernet-кадра */
#define BUF_SIZE 4096

/* Флаг, установленный в 1 в том случае, если целевой сетевой адаптер работает в неразборчивом режиме */
char is_promisc = 0;

/* Номер дескриптора сокета в таблице файловых дескрипторов процесса */
int sock = -1;

/* Описатель параметров сетевого интерфейса */
struct ifreq req;

/* Обработчик сигнала SIGINT, по получению которого главный процесс программы должен корректно завершится */
void SIGINT_handler(int notused)
{
	if(sock != -1)
	{
		/* Если целевой сетевой адаптер работает в неразборчивом режиме, то он переводится обратно в обычный режим функционирования */
		if(is_promisc)
		{
			/* Логическим исключающим ИЛИ снимается установленный флаг IFF_PROMISC */
			req.ifr_flags ^= IFF_PROMISC;

			/* Установ новых значений параметров сетевого адаптера */
			ioctl(sock, SIOCSIFFLAGS, & req);
		}

		/* Уничтожение сокета */
		close(sock);
	}

	printf("\n-----> Останов прослушивания <-----\n\n");

	/* Завершение главного процесса программы */
	exit(0);
}

/* Функция вывода сообщения об ошибки в стандартный поток ошибок */
int error(const char *mes)
{
	fprintf(stderr, "[Ошибка] %s\n\n", mes);
	
	if(sock != -1)
		close(sock);

	return -1;
}

/* Главная функция программы */
int main(const int argc, const char *argv[])
{
	printf("\n");
	
	/* Вывод справочной информации в случае, если программа запущена с некорректным количеством ключей */
	if(argc != 2)
		return error("Программе передано недостаточное количество аргументов\n\nФормат вызова: ./program ИМЯ_ПРОСЛУШИВАЕМОГО_СЕТЕВОГО_ИНТЕРФЕЙСА");

	/* Установ обработчика сигнала SIGINT */
	if(signal(SIGINT, & SIGINT_handler) == SIG_ERR)
		return error("Невозможно установить обработчик сигнала SIGINT");

	/* Создание сырого (SOCK_RAW) пакетного (AF_PACKET) сокета, с помощью которого будут перехвачены Ethernet-кадры, содержащие пакеты протокола IP (ETH_P_IP) */
	sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
	if (sock == -1)
		return error("Невозможно создать сокет");

	/* Помещение в поле ifr_name экземпляра структуры данных ifreq имени сетевого интерфейса целевого сетевого адаптера */
	strcpy(req.ifr_name, argv[1]);

	/* Получение (SIOCGIFFFLAGS) набора флагов, описывающих настройки целевого сетевого адаптера */
	if(ioctl(sock, SIOCGIFFLAGS, & req))
		return error("Невозможно получить набор параметров сетевого адаптера\n");

	/* Установ флага IFF_PROMISC, отвечающего за нахождение целевого сетевого адаптера в неразборчивом режиме */
	req.ifr_flags |= IFF_PROMISC;

	/* Установ (SIOCSIFFLAGS) набора флагов, описывающих настройки целевого сетевого адаптера */
	if(ioctl(sock, SIOCSIFFLAGS, & req))
		return error("Невозможно перевести сетевой адаптер в неразборчивый режим\n");

	/* Флаг нахождения сетевого адаптера в неразборчивом режиме устанавливается в 1 */
	is_promisc = 1;

	char buf[BUF_SIZE + 1];
	unsigned tcp_data_offset;
	ssize_t get_bytes;
	
#define TCP_HEADER_OFFSET (sizeof(struct ethhdr) + sizeof(struct iphdr))

	struct in_addr saddr, daddr;

	/* Указатель на описатель заголовка Ethernet-кадра устанавливается в значение адреса начала буфера */
	struct ethhdr *eth_header = (struct ethhdr *) buf;

	/* Указатель на описатель заголовка IP-пакета устанавливается в значение адреса начала буфера, увеличенное на размер заголовка Ethernet-кадра */
	struct iphdr *ip_header = (struct iphdr *) (buf + sizeof(struct ethhdr));

	/* Указатель на описатель заголовка TCP-пакета устанавливается в значение адреса начала буфера, увеличенное на размеры заголовков Ethernet-кадра и IP-пакета */
	struct tcphdr *tcp_header = (struct tcphdr *) (buf + TCP_HEADER_OFFSET);

	printf("-----> Запуск прослушивания <-----\n\n");

	/* Главный цикл программы */
	while(1)
	{
		if(
			/* Чтение очередного Ethernet-кадра */
			(get_bytes = recv(sock, buf, BUF_SIZE, 0)) > 0
			&&
			/* Если кадр прочитан и в кадре содержится IP-пакет */
			(eth_header->h_proto == htons(ETH_P_IP))
			&&
			/* Если в IP-пакете содержится TCP-пакет */
			(ip_header->protocol == IPPROTO_TCP)
		  )
		{
			saddr.s_addr = ip_header->saddr;
			daddr.s_addr = ip_header->daddr;
			tcp_data_offset = TCP_HEADER_OFFSET + tcp_header->doff * 4;

			/* Вывод информации о полученном Ethernet-кадре (адресы источника и получателя, размеры в байтах Ethernet-кадра и поля данных TCP-пакета) */
			printf("[ %s:%u -> ", inet_ntoa(saddr), ntohs(tcp_header->source));
			printf("%s:%u ] [ %d байт, %d байт данных ] [ ", inet_ntoa(daddr), ntohs(tcp_header->dest), get_bytes, get_bytes - tcp_data_offset);

			/* Вывод информации об установленных флагах TCP-пакета */
			if(tcp_header->fin)
				printf("FIN ");

			if(tcp_header->syn)
				printf("SYN ");

			if(tcp_header->rst)
				printf("RST ");

			if(tcp_header->psh)
				printf("PUSH ");

			if(tcp_header->ack)
				printf("ACK ");

			if(tcp_header->urg)
				printf("URG ");

			printf("] ");

			/* Если поле данных TCP-пакета не пусто */
			if(tcp_data_offset != get_bytes)
			{
				/* Последний байт поля данных TCP-пакета устанавливается в '\0' в целях предотвращения вывода символа перевода строки, возможно находящегося в данном байте из-за специфики работы утилиты ncat, используемой для демонстрации работоспособности сниффера */
				if(buf[get_bytes - 1] == 0x0A)
					buf[get_bytes - 1] = '\0';

				printf("[ %s ]\n", buf + tcp_data_offset);
			}
			else
				printf("\n");
		}
	}

	/* Данная строка никогда не будет достигнута, так как бесконечный главный цикл не может завершится из-за ложности своего условия */
}

