#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

/* 线程函数，新连接的读写处理 */
void* thread_fun(void* arg)
{
	int connfd = *(int*)arg;

	while (1)
	{
		char buffer[128] = { 0 };

		int n = recv(connfd, buffer, 127, 0);
		if (n <= 0) /* recv返回0表示通信对方已经关闭连接 */
		{
			break;
		}
		printf("buffer(%d) = %s\n", n, buffer);
		send(connfd, "OK", 2, 0);
	}
	printf("One client over!\n");
	/* close关闭套接字 */
	close(connfd);
}

int main()
{
	/* 1、通过socket函数创建监听套接字 */
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd != -1);

	/* 定义服务器和客户端的专用socket地址*/
	struct sockaddr_in saddr, caddr;
	memset(&saddr, 0, sizeof(saddr));// 将其内存清空

	/* 设置地址族、端口号、ip地址 */
	saddr.sin_family = AF_INET; // 
	saddr.sin_port = htons(6000);
	saddr.sin_addr.s_addr = inet_addr("192.168.204.129");

	/* 2、通过bind将套接字与socket地址绑定起来 */
	int res = bind(sockfd, (struct sockaddr*) & saddr, sizeof(saddr));
	assert(res != -1);

	/* 3、创建监听队列，监听队列长度设为5 */
	listen(sockfd, 5);

	while (1)
	{
		int len = sizeof(caddr);

		/*
		** accept监听socket，若存在，则从监听队列中接受一个连接
		** 函数返回一个链接套接字，否则，没有新连接请求，则阻塞等待
		*/
		int connfd = accept(sockfd, (struct sockaddr*) & caddr, (socklen_t*)& len);

		/* accept失败返回-1表示没有获取到连接，继续循环 */
		if (connfd < 0)
		{
			continue;
		}

		printf("accept connfd = %d, ip = %s, port = %d\n",
			connfd, inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));

		pthread_t id; // 定义线程号
		/* 创建新线程，将连接套接字connfd作为参数传入 */
		pthread_create(&id, NULL, thread_fun, (void*)& connfd);
	}
	close(sockfd);
}
