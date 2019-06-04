#include "socket.h"
#include "thread.h"

int main()
{
	/* 获取到已创建的套接字描述符 */
	int sockfd = create_socket();
	assert(sockfd != -1);

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
		printf("accept connfd : %d\n", connfd);
		int res = thread_start(connfd);
		printf("res = %d",res );
        if (res == -1)
		{
			close(connfd);
		}
	}
}
