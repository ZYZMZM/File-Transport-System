/********************************************************************************
 * File name: socket.c
 * Description: 本文件只封装了一个API接口 create_socket(),create_socket函数负责
 *              创建套接字、命名套接字、创建监听队列等操作，返回创建成功的socket
 *              描述符
 * Author: ZYZMZM
 * Date: 10/05/2019
 * Version: v1.0
 * History:
 * *******************************************************************************/
#include "socket.h"

#define PORT 6000
#define IPSTR "192.168.204.129"
#define LIS_MAX 100
#define ERROR -1

int create_socket()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == ERROR)
	{
		return ERROR;
	}

	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(PORT);
	saddr.sin_addr.s_addr = inet_addr(IPSTR);

	int res = bind(sockfd, (struct sockaddr*) & saddr, sizeof(saddr));


	if (res == ERROR)
	{
		return ERROR;
	}

	listen(sockfd, LIS_MAX);

	return sockfd;
}
