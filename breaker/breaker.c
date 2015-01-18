/*********************************************************
*	File Name   : breaker.c
*	Project     : 3389defender
*	Author      : Kent
*	Data        : 2014年04月06日 星期日 16时39分53秒
*	Description : 尝试不断的连接3389端口
*	              
*	History     : 
*		2014年04月06日 星期日 16时39分53秒
*			Create.
**********************************************************/
/* Public headers */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/timerfd.h>
#include <arpa/inet.h>

/* 测试用log宏 */
#ifdef __TEST__
#define LOG(x, ...) printf(x, ##__VA_ARGS__);
#else
#define LOG(x, ...)
#endif

/* 最大连接数以及端口 */
#ifndef MAX
#define MAX 3000
#endif
#ifndef PORT
#define PORT 3389
#endif

/*********************************************************
*	Func Name   : main
*	Project     : 3389defender
*	Author      : Kent
*	Data        : 2014年04月06日 星期日 16时42分09秒
*	Description : main function
*	              
*	History     : 
*		2014年04月06日 星期日 16时42分09秒
*			Create.
*		2014年04月06日 星期日 16时57分42秒
*			Fisrt version compile OK
*		2014年04月06日 星期日 17时17分20秒
*			debug 1st OK.
**********************************************************/
int main(int argc, char **argv)
{
	int maxconnect = 0;
	int sockfd;
	int ret;
	int count;
	struct sockaddr_in targethost;

	/* check command input */
	if (3 != argc)
	{
		printf("Usage: ./3389breaker x.x.x.x 3000\n");
		return -1;
	}
	ret = sscanf(argv[2], "%d", &maxconnect);
	if (MAX < maxconnect)
	{
		printf("Maxconnect is limited.Nedd change the connection count.\n");
		return -1;
	}
	memset(&targethost, 0, sizeof(targethost));
	targethost.sin_family = AF_INET;
	ret = inet_pton(AF_INET, argv[1], &targethost.sin_addr);
	if (0 >= ret)
	{
		printf("Target Host ip false.\n");
		return -1;
	}
	targethost.sin_port = htons(PORT);
	printf("IP is %s ,count is %d, Port is %d, Ready?\n",argv[1], maxconnect, ntohs(targethost.sin_port));
	getchar();

	/* 创建socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > sockfd)
	{
		LOG("ERROR: SOCKET Create Failed.\n");
		return -1;
	}

	/* try connect */
	for (count = 1; count <= maxconnect; count++)
	{
		printf("Try No.%d connection :", count);
		fflush(stdout);
		ret = connect(sockfd, (struct sockaddr *)&targethost, sizeof(targethost));
		if (0 != ret)
		{
			perror("\nFailed:");
			break;
		}
		printf("Done.\n");
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
	}

	printf("All is Done.\n");
	getchar();

	return 0;
}
