/********************************************************************************
 * File name: ser.c
 * Description: 该文件时系统的主程序文件，但不是主功能实现文件，该文件创建监听
 *              套接字，并根据是否有新连接去调用线程创建函数开启新客户端的交互。
 *              每次服务器启动，都调用脚本文件进行服务器启动日志记录，并根据服
 *              务器是否是首次启动(API)，若是则调用相应脚本文件信息数据表的初始
 *              化操作。
 * Author: ZYZMZM
 * Date: 02/06/2019
 * Version: v12.0
 * History:
 * *******************************************************************************/
#include "socket.h"
#include "thread.h"
#include "user.h"
#define ERROR -1

int main()
{
	/* 获取到已创建的套接字描述符 */
	int sockfd = create_socket();
	assert(sockfd != ERROR);

	pid_t pid = fork();
	assert(pid != ERROR);

	if (pid == 0)
	{
		execl("./server_start_info.sh", "server_start_info.sh", (char*)0);
	}
	wait(NULL);


	/* 服务器是否首次启动 ，首次启动对数据库进行初始化*/
	if (initStart())
	{
		pid_t pid = fork();
		assert(pid != ERROR);

		if (pid == 0)
		{
			execl("./fileMd5.sh", "fileMd5.sh", (char*)0);
		}
		wait(NULL);
	}

	while (1)
	{
		struct sockaddr_in caddr;
		int len = sizeof(caddr);

		/* accept接收新的客户连接 */
		int connfd = accept(sockfd, (struct sockaddr*) & caddr, (socklen_t*)& len);

		/* 没有新连接则继续循环监听 */
		if (connfd <= 0)
		{
			continue;
		}
		printf("new client enter : %d\n", connfd);
		int res = thread_start(connfd);
		if (res == ERROR)
		{
			close(connfd);
		}
	}
}
