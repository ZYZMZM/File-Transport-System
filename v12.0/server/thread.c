/********************************************************************************
 * File name: thread.c
 * Description: 该函数是整个服务器系统的核心文件，基本所有的交互功能、数据库
 *              表修改、初始化等功能，大文件的上传、下载、断点续传、秒传等功能都
 *              在这里实现，文章中调用了很多shell脚本程序来完成一些复杂的操作，
 *              调用了user.h数据库操作函数，该文件中封装了大量与本系统数据库进行
 *              交互的API接口，在本函数中直接调用，与数据库继续交互。
 * Author: ZYZMZM
 * Date: 02/06/2019
 * Version: v12.0
 * *******************************************************************************/
#define _GNU_SOURCE 1
#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64
#endif

#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64
#endif

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#include "thread.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <sys/stat.h>
#include "user.h"
#include <openssl/md5.h>

#define ARGC 10
#define READ_BUFF BUF_SIZE
#define BUF_SIZE 4096
#define BUFFER_SIZE 128
#define N BUFFER_SIZE
#define ERROR -1
#define SUCCESS 0
#define NEW_FILE 1
#define CON_FILE 2
#define NOMOD_FILE 3

void* work_thread(void* arg);


/*****************************************************************************************   
 * Function:       thread_start()
 * Description:    本函数主要功能是创建新线程，多客户对服务器的并发访问，ser.c文件中
 *                 监测到有连接请求，便调用该函数进行新线程的创建，即每个客户端都是一个
 *                 线程在服务器中运行，互不干扰，基本满足局域网内的高并发访问。
 * Calls:          work_thread()
 * Called By:      ser.c - main()
 * Table Accessed: NULL
 * Table Updated:  NULL
 * Other:          该函数创建新线程，标识新客户端，线程函数为work_thread(),该客户端的所有
 *                 请求都在线程函数中接收。
*****************************************************************************************/
int thread_start(int connfd)
{
	pthread_t id; // 定义线程号
	/* 创建新线程，将连接套接字connfd作为参数传入 */
	int res = pthread_create(&id, NULL, work_thread, (void*)connfd);
	if (res != 0)
	{
		return ERROR;
	}
	return SUCCESS;
}

void getFile(int connfd, char* filename, char* UserName);
void putFile(int connfd, char* filename, char* UserName);

/*****************************************************************************************   
 * Function:       do_request()
 * Description:    本函数为客端主程序, 负责创建连接套接字、用户的主界面显示、
 *                 处理用户的输入信息、并将输入信息进行本地判定或者发送给服务器等功能
 * Calls:          deleteMd5Info() - 客户端root用户删除文件时，调用此API删除库中数据
 *                 Register() - 用户注册    Match() - 用户登录
 *                 putFile()、getFile()
 * Called By:      thread_start()
 * Table Accessed: md5table(DATABASE: md5), user(DATABASE: loginUser)
 * Table Updated:  md5table(DATABASE: md5),当root用户执行rm命令时，客端将命令完整的
 *                 发送给服务器，服务器将进行数据表中该记录的删除操作。
 *                 user(DATABASE: loginUser),客端进行注册或者登录操作，服务端要在用户
 *                 信息表user中进行注册插入或登录查询验证等操作。
*****************************************************************************************/
void* work_thread(void* arg)
{
	int connfd = (int)arg;
    
    char portbuffer[BUFFER_SIZE] = {0};
    sprintf(portbuffer, "%d", connfd);
    send(connfd, portbuffer, BUFFER_SIZE, 0);

	char UserName[BUFFER_SIZE] = { 0 };
	
	int initFlag = 0;

	while (1)
	{
		if (initFlag == 0)
		{
			char recvBuffer[BUFFER_SIZE] = { 0 };

			int n = recv(connfd, recvBuffer, BUFFER_SIZE, 0);
			if (strncmp(recvBuffer, "1", 1) == 0)
			{
				initFlag = 1;
				strcpy(recvBuffer, "********Register********");
				send(connfd, recvBuffer, BUFFER_SIZE, 0);
			}
			else if (strncmp(recvBuffer, "2", 1) == 0)
			{
				initFlag = 2;
				strcpy(recvBuffer, "********Login********");
				send(connfd, recvBuffer, BUFFER_SIZE, 0);
			}
		}

		if (initFlag == 1)
		{
			char username[BUFFER_SIZE] = { 0 };
			char password[BUFFER_SIZE] = { 0 };
			char recvBuffer[BUFFER_SIZE] = { 0 };

			recv(connfd, recvBuffer, BUFFER_SIZE, 0);
			strcpy(username, recvBuffer);
			strcpy(recvBuffer, "OK");
			send(connfd, recvBuffer, BUFFER_SIZE, 0);

			recv(connfd, recvBuffer, BUFFER_SIZE, 0);
			strcpy(password, recvBuffer);

			int res = Register(username, password);
			if (res == 1)
			{
				strcpy(recvBuffer, "1");
				send(connfd, recvBuffer, BUFFER_SIZE, 0);
				initFlag = 2;
			}
			else
			{
				strcpy(recvBuffer, "-1");
				send(connfd, recvBuffer, BUFFER_SIZE, 0);
				initFlag = 1;
				continue;
			}
		}

		if (initFlag == 2)
		{
			char username[BUFFER_SIZE] = { 0 };
			char password[BUFFER_SIZE] = { 0 };
			char recvBuffer[BUFFER_SIZE] = { 0 };

			recv(connfd, recvBuffer, BUFFER_SIZE, 0);
			strcpy(username, recvBuffer);
			strcpy(recvBuffer, "OK");
			send(connfd, recvBuffer, BUFFER_SIZE, 0);

			recv(connfd, recvBuffer, BUFFER_SIZE, 0);
			strcpy(password, recvBuffer);

			int res = Match(username, password);
			if (res == 1)
			{
				strcpy(recvBuffer, "1");
				send(connfd, recvBuffer, BUFFER_SIZE, 0);
				strcpy(UserName, username);
				break;
			}
			else
			{
				strcpy(recvBuffer, "-1");
				send(connfd, recvBuffer, BUFFER_SIZE, 0);
				initFlag = 2;
				continue;
			}
		}
	}

	while (1)
	{
		char buffer[BUFFER_SIZE] = { 0 };

		int n = recv(connfd, buffer, BUFFER_SIZE, 0);
		if (strncmp(buffer, "close", 5) == 0)
		{
			printf("\nThe Server is Closing\n");
			exit(0);
		}
		if (n <= 0) /* recv返回0表示通信对方已经关闭连接 */
		{
			break;
		}

		int i = 0;
		char* myargv[ARGC] = { 0 };
		char* ptr = NULL;
		char* s = strtok_r(buffer, " ", &ptr);
		while (s != NULL)
		{
			myargv[i++] = s;
			s = strtok_r(NULL, " ", &ptr);
		}
		char* cmd = myargv[0]; // 命令
		if (cmd == NULL)
		{
			send(connfd, "error!", 6, 0);
			continue;
		}
		if (strcmp(cmd, "get") == 0)
		{
			//下载
			getFile(connfd, myargv[1], UserName);
			continue;
		}
		else if (strcmp(cmd, "put") == 0)
		{
			//上传
			putFile(connfd, myargv[1], UserName);
			continue;
		}
		else
		{
			if (strcmp(cmd, "rm") == 0)
			{
				printf("rm :  %s\n", myargv[1]);
				deleteMd5Info(myargv[1]);
			}

			printf("cmd = %s\n", cmd);
			int pipefd[2];
			pipe(pipefd);// 创建管道
			pid_t pid = fork();
			if (pid == -1)
			{
				send(connfd, "error!", 6, 0);
				continue;
			}
			if (pid == 0)
			{
				dup2(pipefd[1], 1);
				dup2(pipefd[1], 2);
				execvp(cmd, myargv);
				perror("cmd:error");
				//exec
				exit(0);
			}
			close(pipefd[1]); // 保证子进程结束后管道的写端彻底关闭，防止cmd为空时read不阻塞
			wait(NULL);

			ssize_t bytes;
			char readbuff[READ_BUFF] = { 0 };
			while ((bytes = read(pipefd[0], readbuff, READ_BUFF)) > 0)
			{
				send(connfd, readbuff, bytes, 0);
			}

			sleep(1);

			strcpy(readbuff, "over");
			send(connfd, readbuff, READ_BUFF, 0);
			printf("send over\n");
			close(pipefd[0]);
		}
	}
	printf("One client over!\n");
	/* close关闭套接字 */
	close(connfd);
}


/********************************************************************************************   
 * Function:       getFile()
 * Description:    本函数实现客户端请求下载文件，服务器发送该文件的功能
 *                 实现大文件的传输、断点续传、秒传等功能。
 * Calls:          isIntact() - 在数据表md5table中根据标志位判断文件是否完整
 * Called By:      work_thread()
 * Table Accessed: md5table(DATABASE: md5),还函数在md5table中检查该文件完整
 *                 标志位，若标志位为0，这说明文件不完整，向客户端发送错误码，
 *                 不允许客户端下载该文件
 * Table Updated:  md5table(DATABASE: md5)
 * Input:          socket - 连接套接字   filename - 待上传的文件名 
 *                 username - 当前登录的用户
 * Output:         NULL   
 * Return:         NULL
 * Others:         本函数除了基本的传输功能外，也实现了大文件的传输、断点续传、
 *                 秒传等功能，其中大文件下载要使用open函数的O_LARGEFILE标志位，
 *                 lseek64\ftruncate64等函数实现。文件大小都要使用long long 类型。
 *                 注意断点续传功能，我们直接根据文件大小进行续传，因为我们默认
 *                 服务器是不会更改用户上传的文件的。
 *                 那么当客户端因为种种问题文件没有传输完毕，那么下次直接从
 *                 断点续传即可，不需要匹配，因为服务器从没改过该文件。
 *                 为了解决客户端在传输过程中异常中断的情况，我们可以使用两种
 *                 方法使得服务正确处理SIGPIPE信号，一是使用send函数的MSG_NOSIGNAL
 *                 标志位，第二种方法是使用poll函数监测POLLRDHUP事件，关闭连接套接
 *                 字，使得服务器正确处理异常中断情况而不会崩溃退出
********************************************************************************************/
void getFile(int connfd, char* filename, char* UserName)
{
	int fd;
	char buffer[BUFFER_SIZE] = {};

	long long bytes;
	if ((fd = open(filename, O_RDONLY | O_LARGEFILE)) < 0)
	{
		if (errno == ENOENT)
		{
			strcpy(buffer, "NOT EXISTS");
			send(connfd, buffer, BUFFER_SIZE, 0);
			return;
		}
		else
		{
			printf("fail to open");
		}
	}
	else
	{
		if (isIntact(filename) == 0)
		{
			strcpy(buffer, "ERROR");
			send(connfd, buffer, BUFFER_SIZE, 0);
			return;
		}
	}

	char tmp[BUFFER_SIZE] = { 0 };
	long long size = lseek64(fd, 0, SEEK_END);
	lseek64(fd, 0, SEEK_SET);

	sprintf(tmp, "%lld", size);
	strcpy(buffer, tmp);
	send(connfd, buffer, BUFFER_SIZE, 0);
	printf("send size = %s\n", buffer);

	sleep(1);

	struct pollfd fds[1];
	fds[0].fd = connfd;
	fds[0].events = POLLRDHUP;
	fds[0].revents = 0;

	char buf[BUF_SIZE] = { 0 };
	recv(connfd, buf, BUF_SIZE, 0);
	if ((strncmp(buf, "##over##", 8) == 0))
	{
		return;
	}
	else if (strncmp(buf, "OK", 2) == 0)
	{
		while ((bytes = read(fd, buf, BUF_SIZE)) > 0)
		{
			int n = poll(fds, 1, 0);
			if (n < 0)
			{
				perror("poll fail\n");
				exit(0);
			}
			if (fds[0].revents & POLLRDHUP)
			{
				printf("客户端异常中断\n");
				close(connfd);
				return;
			}
			//send(connfd, buf, bytes, MSG_NOSIGNAL);
			send(connfd, buf, bytes, 0);
		}
	}
	else
	{
		long long index = 0;
		index = atoll(buf);
		printf("index=%lld\n", index);
		long long num = lseek64(fd, index, SEEK_SET);
		printf("num = %lld\n", num);
		num = 0;
		while ((bytes = read(fd, buf, BUF_SIZE)) > 0)
		{
			int n = poll(fds, 1, 0);
			if (n < 0)
			{
				perror("poll fail\n");
				exit(0);
			}
			if (fds[0].revents & POLLRDHUP)
			{
				printf("客户端异常中断\n");
				close(connfd);
				return;
			}
			num += bytes;
			//send(connfd, buf, bytes, MSG_NOSIGNAL);
			send(connfd, buf, bytes, 0);
		}
		printf("num = %lld\n", num);
	}

	sleep(1);

	strcpy(buf, "##over##");
	//send(connfd, buffer, BUFFER_SIZE, MSG_NOSIGNAL);
	send(connfd, buf, BUF_SIZE, 0);

	printf("send over\n");
}

/******************************************************************************************
 * Function:       putFile()
 * Description:    本函数实现客户端要求上传文件，服务器进行文件接收的功能
 * Calls:          matchMd5() - 秒传信息匹配
 *                 isIntact() - 根据标志位判断文件是否完整
 *                 isVaildUser() - 判断上传文件的用户是否和库中该文件所属者是否一致
 *                 insertMd5Info() - 将上传的文件信息插入到数据表md5table中
 *                 setIntact() - 设置文件完整标志位
 * Called By:      work_thread()
 * Table Accessed: md5table(DATABASE: md5),访问该表主要进行秒传信息的匹配，判断文件的
 *                 完整与否、判断文件所属信息与当前登录用户的匹配等
 * Table Updated:  md5table(DATABASE: md5),操作该数据库主要是添加文件信息，在客户开始
 *                 上传前就将此文件的相关信息(文件名、文件md5值、所属用户、完整程度)
 *                 存入数据库中，完整标志位置为0; 当客端上传完毕后，setIntact()将该文件
 *                 标志位置为1，表示文件完整，其他任何用户便可以下载该文件。
 * Input:          connfd - 连接套接字   filename - 待上传的文件名 
 *                 username - 当前登录用户
 * Output:         NULL
 * Return:         NULL
 * Others:         本函数除了基本的上传功能外，也实现了大文件的接收、断点续传、秒传功能，
 *                 其中大文件传输要使用open函数的O_LARGEFILE标志位，lseek64\ftruncate64等
 *                 函数实现。文件大小都要使用long long 类型。
 *                 注意断点续传功能，必须是相同用户登录才能实现断点续传功能，
 *                 其他用户都是重传文件。这也符合实际的需求。
 *                 为了解决客户端在传输过程中异常中断的情况，我们使用poll函数监测
 *                 POLLRDHUP事件，关闭连接套接字，使得服务器正确处理异常中断情况而
 *                 不会崩溃退出
*******************************************************************************************/
void putFile(int connfd, char* filename, char* UserName)
{
	char buffer[BUFFER_SIZE] = { 0 };
	char md5value[BUFFER_SIZE] = { 0 };
	int fd;
	long long bytes;
	int flag = 0;

	/* 发begin */
	strcpy(buffer, "Begin");
	send(connfd, buffer, BUFFER_SIZE, 0);

	recv(connfd, md5value, BUFFER_SIZE, 0);
	md5value[32] = '\0'; // 顺序不能动
	int match = matchMd5(filename, md5value);
    printf("buffer = %s\n", md5value);
    printf("match = %d\n", match);

	if (match == 1 && isIntact(filename))
	{
		strcpy(buffer, "exist");
		send(connfd, buffer, BUFFER_SIZE, 0);
		recv(connfd, buffer, BUFFER_SIZE, 0);
		if (strncmp(buffer, "over", 4) == 0)
		{
			printf("秒传完毕\n");
			return;
		}
	}
	else
	{
		strcpy(buffer, "no");
		send(connfd, buffer, BUFFER_SIZE, 0);
		recv(connfd, buffer, BUFFER_SIZE, 0);
	}

	strcpy(buffer, "begin");
	send(connfd, buffer, BUFFER_SIZE, 0);

	//=========================

	char buf[BUF_SIZE] = { 0 };
	recv(connfd, buf, BUF_SIZE, 0);
	printf("File Size(from client):%s\n", buf);
	long long Osize = atoll(buf);

	long long size = 0;
	if ((fd = open(filename, O_RDWR | O_LARGEFILE)) < 0)
	{
		if ((fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC | O_LARGEFILE, 0664)) < 0)
		{
			printf("fail to open\n");
		}
		flag = NEW_FILE;
	}
	else if (isVaildUser(filename, UserName) == 0)//新用户
	{
		flag = NEW_FILE;
	}
	else
	{
		size = lseek64(fd, 0, SEEK_END);

		if (Osize <= size)
		{
			flag = NEW_FILE;
			ftruncate64(fd, 0);
			lseek64(fd, 0, SEEK_SET);
		}
		else if (Osize > size)
		{
			int intact = isIntact(filename);
			if (intact == 0)//不完整
			{
				flag = CON_FILE;

				printf("File Size(from local) = %d\n", size);
			}
			else
			{
				flag = NEW_FILE;
				ftruncate64(fd, 0);
				lseek64(fd, 0, SEEK_SET);
			}
		}
	}

	struct pollfd fds[1];
	fds[0].fd = connfd;
	fds[0].events = POLLRDHUP;
	fds[0].revents = 0;

	if (flag == NEW_FILE)
	{
		insertMd5Info(filename, md5value, UserName);
		strcpy(buf, "OK");
		send(connfd, buf, BUF_SIZE, 0);
		printf("正在下载%s,请稍候\n", filename);
		while ((bytes = recv(connfd, buf, BUF_SIZE, 0)) > 0)
		{
			int n = poll(fds, 1, 0);
			if (n < 0)
			{
				perror("poll fail\n");
				exit(0);
			}
			if (fds[0].revents & POLLRDHUP)
			{
				printf("客户端异常中断\n");
				close(connfd);
				return;
			}
			if (strncmp(buf, "##over##", 8) == 0)
			{
				break;
			}
			write(fd, buf, bytes);
		}
	}

	else if (flag == CON_FILE)
	{
		char sizebuff[BUF_SIZE] = { 0 };
		sprintf(sizebuff, "%lld", size);
		send(connfd, sizebuff, BUF_SIZE, 0);

		long long num = lseek64(fd, size, SEEK_SET);
		printf("Origin Size = %lld\n", num);
		num = 0;
		while ((bytes = recv(connfd, buf, BUF_SIZE, 0)) > 0)
		{
			int n = poll(fds, 1, 0);
			if (n < 0)
			{
				perror("poll fail\n");
				exit(0);
			}
			if (fds[0].revents & POLLRDHUP)
			{
				printf("客户端异常中断\n");
				close(connfd);
				return;
			}
			if (strncmp(buf, "##over##", 8) == 0)
			{
				break;
			}
			num += write(fd, buf, bytes);
		}
		printf("Continue Transport Size = %lld\n", num);
	}
	close(fd);
	struct stat64 st;
	stat64(filename, &st);
	setIntact(filename, "1");
	printf("%s下载完毕，大小为%lld\n", filename, st.st_size);
}
