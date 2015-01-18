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

/* 端口声明改到makefile中 */
#ifndef LISTENPORT
#define LISTENPORT 3389
#endif

/* 函数声明 */
static int main_loop(FILE *recordfile);

/*********************************************************
*	Func Name   : main
*	Project     : Rasp_Driver
*	Author      : Kent
*	Data        : 2014年04月03日 星期四 16时23分31秒
*	Description : 主函数
*	              
*	History     : 
*		2014年04月03日 星期四 16时23分31秒
*			Create.
*		2014年04月03日 星期四 21时59分38秒
*			Finish first version.
*		2014年05月14日 星期三 16时53分22秒
*			check splint result
**********************************************************/
int main()
{
	int ret = 0;
	pid_t ppd = 0;
	FILE *recordfile = NULL;
	FILE *pidfile = NULL;

	/* 打开记录文件 */
	(void)system("mkdir ./record");
	(void)system("touch ./record/ip.txt");
	recordfile = fopen("./record/ip.txt","a");
	if (NULL == recordfile)
	{
		perror("Open File:");
		return 0;
	}
	LOG("Open file successed.\n");

	/* 打开进程pid记录文件 */
	(void)system("touch /var/run/3389def.pid");
	pidfile = fopen("/var/run/3389def.pid","w+");
	if (NULL == pidfile)
	{
		perror("Open pidfile:");
		return 0;
	}

#ifndef __TEST__
	/* 创建守护进程 */
	ppd = fork();
	if (0 > ppd)
	{
		printf("Daemon failed.\n");
		goto __exit;
	}
	if (0 == ppd)
	{
		if (0 > setsid())
		{
			perror("Setsed:");
			return 0;
		}
	}
	if (0 < ppd)
	{
		/* 主进程退出 */
		fprintf(pidfile, "%d", (int)ppd);
		return 0;
	}
#endif

	/* 进入主循环 */
	ret = main_loop(recordfile);
	ret = ret;

	/* exit */
__exit:
	(void)fclose(recordfile);
	return 0;
}

/*********************************************************
*	Func Name   : main_loop
*	Project     : 3389defender
*	Author      : Kent
*	Data        : 2014年04月03日 星期四 21时53分55秒
*	Description : 记录clien的ip地址的主循环
*	              
*	History     : 
*		2014年04月03日 星期四 21时53分55秒
*			Create.
*		2014年04月03日 星期四 22时50分58秒
*			Finish first version.
*		2014年04月04日 星期五 13时34分33秒
*			add record failed process.
*		2014年05月14日 星期三 16时53分08秒
*			check splint result
**********************************************************/
static int main_loop(FILE *recordfile)
{
	int ret = 0;
	int sockfd = -1;
	int clientfd = -1;
	int on = 1;
	struct sockaddr_in hostaddr;
	struct sockaddr_in clientaddr;
	socklen_t c_addr_len;
	char ipaddr[100] = "";
	time_t rctime = -1;
	char *timestr = NULL;

	/* 创建socket，并且进行端口重用 */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > sockfd)
	{
		LOG("ERROR: SOCKET Create Failed.\n");
		return -1;
	}
	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, (socklen_t)sizeof(on));
	if (0 > ret)
	{
		LOG("REUSE ADDR AND PORT ERROR.\n");
		(void)close(sockfd);
		return -1;
	}

	/* 设置IPv4本机地址 */
	memset(&hostaddr, 0, sizeof(hostaddr));
	hostaddr.sin_family = (in_addr_t)AF_INET;
	hostaddr.sin_port = htons(LISTENPORT);
	/* 绑定socket */
	ret = bind(sockfd, (struct sockaddr *)&hostaddr, (socklen_t)sizeof(hostaddr));
	if (0 > ret)
	{
		LOG("ERROR: BIND PORT ERROR.\n");
		perror("BIND:");
		(void)close(sockfd);
		return -1;
	}
	LOG("PORT is : %d\n", LISTENPORT);

	/* 开始监听端口 */
	ret = listen(sockfd, 20);
	if (0 > ret)
	{
		LOG("ERROR: LISTEN Failed.\n");
		(void)close(sockfd);
		return -1;
	}

	/* 循环记录请求连接的客户端ip地址 */
	while (1)
	{
		memset(&clientaddr, 0, sizeof(clientaddr));
		memset(&ipaddr, 0, sizeof(ipaddr));
		c_addr_len = (socklen_t)sizeof(clientaddr);
		clientfd = accept(sockfd, (struct sockaddr *)&clientaddr, &c_addr_len);
		if (0 > clientfd)
		{
			LOG("accept connection failed.\n");
			perror("accept:");
			(void)sleep(10);
			continue;
		}
		
		/* 记录获取的地址以及时间 */
		inet_ntop(AF_INET, &clientaddr.sin_addr, ipaddr, 100);
		rctime = time(NULL);
		if (0 > rctime)
		{
			LOG("Get Time failed.\n");
			break;
		}
		timestr = ctime(&rctime);
		ret = fprintf(recordfile, "IP is: %s \t\t Time is : %s", ipaddr, timestr);
		if (0 >= ret)
		{
			/* 重新打开记录文件 */
			LOG("Write File Error.\n");
			(void)system("mkdir ./record");
			(void)system("touch ./record/ip.txt");
			recordfile = fopen("./record/ip.txt","a");
			if (NULL == recordfile)
			{
				perror("Open File:");
				return -1;
			}
			ret = fprintf(recordfile, "IP is: %s \t\t Time is : %s", ipaddr, timestr);
			if (0 >= ret)
			{
				LOG("Record failed process failed.\n");
				return -1;
			}
		}
		(void)fflush(recordfile);
		LOG("Record Successed.\n");

		/* 此时断开客户端 */
		(void)close(clientfd);
	}

	return -1;
}
