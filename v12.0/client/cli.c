/*****************************************************************************************
 * File name: cli.c
 * Description: 此程序文件主要完成客户端与服务器的交互功能
 *              用户的登录与注册、基本命令的执行、文件的上传与下载、秒传与断点续传等客端
 *              操作及界面显示，调用了外部文件printHelpInfo.h，其中封装了3个打印不同帮助
 *              信息的API接口，均在本文件中有调用，本文件也使用到了两个外部的shell脚本文
 *              件：com.sh和main.sh,作用分别是接收服务器传递的命令结果、计算客端上传文件
 *              的MD5值以便传递给服务器。
 * Author: ZYZMZM
 * Date: 02/06/2019
 * Version: v12.0
 * **************************************************************************************/
#define _GNU_SOURCE
#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64
#endif

#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64
#endif

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <readline/history.h>
#include <readline/readline.h>
#include "printHelpInfo.h"


#define N BUFFER_SIZE
#define PORT 6000
#define IPSTR "192.168.204.129"
#define LIS_MAX 20
#define BUFFER_SIZE 128
#define BUF_SIZE 4096
#define NEW_FILE 1
#define CON_FILE 2
#define NOMOD_FILE 3

int create_to_ser();

void getFile(int sockfd, char* filename);

void putFile(int sockfd, char* filename);

void do_request();

int Identity = 0;


int main()
{
	do_request();
}


/************************************************************************************************   
 * Function:       do_request()
 * Description:    本函数为客端主程序, 负责创建连接套接字、用户的主界面显示、
 *                 处理用户的输入信息、并将输入信息进行本地判定或者发送给服务器等功能
 * Calls:          create_to_ser()、startHelpInfo()、rootHelpInfo()、userHelpInfo()、
 *                 putFile()、getFile()
 * Called By:      main()
 * Table Accessed: md5table(DATABASE: md5)
 * Table Updated:  md5table(DATABASE: md5),当root用户执行rm命令时，客端将命令完整的
 *                 发送给服务器，服务器将进行数据表中该记录的删除操作。
 * Other    :      程序首先建立与客端的连接，然后获取到客端发送的连接套接字值，便于在
 *                 启动信息中反馈给用户，接下来是用户管理部分，用户进行注册或者登录操作
 *                 将输入信息传递给服务器并进行验证，根据服务器的反馈进行相应操作。
 *                 用户登录成功后，便开始循环监测用户输入的命令，根据相应的命令进行操作。
************************************************************************************************/
void do_request()
{
	int sockfd = create_to_ser();
	assert(sockfd != -1);
    
    char portBuffer[BUFFER_SIZE] = {0};
    recv(sockfd, portBuffer, BUFFER_SIZE, 0); 

	int initFlag = 0;

	while (1)
	{
		if (initFlag == 0)
		{
			printf("please input your choice ：0(login)  1(register) ");
			system("clear");
			char input[BUFFER_SIZE] = { 0 };
			fgets(input, BUFFER_SIZE, stdin);
			input[strlen(input) - 1] = 0;
			if ((strncmp(input, "1", 1) != 0)
				&& (strncmp(input, "0", 1) != 0))
			{
				printf("Input invaild\n");
				system("clear");
				continue;
			}
			else if (strncmp(input, "1", 1) == 0)
			{
				initFlag = 1;
				strcpy(input, "1");
				send(sockfd, input, BUFFER_SIZE, 0);
				recv(sockfd, input, BUFFER_SIZE, 0);
				printf("%s\n", input);
			}
			else if (strncmp(input, "0", 1) == 0)
			{
				initFlag = 2;
				strcpy(input, "2");
				send(sockfd, input, BUFFER_SIZE, 0);
				recv(sockfd, input, BUFFER_SIZE, 0);
				printf("%s\n", input);
			}
		}

		if (initFlag == 1)
		{
			printf("username : ");
			char username[BUFFER_SIZE] = { 0 };
			fgets(username, BUFFER_SIZE, stdin);
			username[strlen(username) - 1] = 0;
			send(sockfd, username, BUFFER_SIZE, 0);
			recv(sockfd, username, BUFFER_SIZE, 0);

			struct termios init, new;
			char password[BUFFER_SIZE] = { 0 };
			tcgetattr(fileno(stdin), &init);
			new = init;
			new.c_lflag &= ~ECHO;
			printf("password : ");
			if (tcsetattr(fileno(stdin), TCSAFLUSH, &new) != 0)
			{
				fprintf(stderr, "error\n");
			}
			else
			{
				fgets(password, BUFFER_SIZE, stdin);
				tcsetattr(fileno(stdin), TCSANOW, &init);
			}
			password[strlen(password) - 1] = 0;
			send(sockfd, password, BUFFER_SIZE, 0);

			char res[BUFFER_SIZE] = { 0 };
			recv(sockfd, res, BUFFER_SIZE, 0);
			if (strncmp(res, "1", 1) == 0)
			{
				system("clear");
				printf("Register success, please login\n");
				initFlag = 2;
			}
			else
			{
				system("clear");
				printf("The user already exists, Please change other usernames\n");
				initFlag = 1;
				continue;
			}
		}

		if (initFlag == 2)
		{
			char recvBuffer[BUFFER_SIZE] = { 0 };

			printf("username : ");
			char username[BUFFER_SIZE] = { 0 };
			fgets(username, BUFFER_SIZE, stdin);
			username[strlen(username) - 1] = 0;
			send(sockfd, username, BUFFER_SIZE, 0);
			recv(sockfd, recvBuffer, BUFFER_SIZE, 0);

			struct termios init, new;
			char password[BUFFER_SIZE] = { 0 };
			tcgetattr(fileno(stdin), &init);
			new = init;
			new.c_lflag &= ~ECHO;
			printf("password : ");
			if (tcsetattr(fileno(stdin), TCSAFLUSH, &new) != 0)
			{
				fprintf(stderr, "error\n");
			}
			else
			{
				fgets(password, BUFFER_SIZE, stdin);
				tcsetattr(fileno(stdin), TCSANOW, &init);
			}
			password[strlen(password) - 1] = 0;
			send(sockfd, password, BUFFER_SIZE, 0);


			char res[BUFFER_SIZE] = { 0 };
			recv(sockfd, res, BUFFER_SIZE, 0);
			if (strncmp(res, "1", 1) == 0)
			{
				system("clear");
				printf("%s Login Success\n", username);
				if (strcmp(username, "root") == 0)
					Identity = 1;
				
                startHelpInfo(portBuffer);

                break;

			}
			else
			{
				system("clear");
				printf("Username or password incorrect\n");
				initFlag = 2;
				continue;
			}
		}
	}

	while (1)
	{
		char buffer[BUFFER_SIZE] = { 0 };
		char* str = "Connect> ";
		char* p = readline(str);	
		add_history(p);
		strcpy(buffer, p);
		if (buffer[0] == 0 || buffer[0] == ' ')
		{
			continue;
		}
		else
		{
			if (strncmp(buffer, "get", 3) == 0)
			{
				if (strlen(buffer) == 3 || strlen(buffer) == 4)
				{
					continue;
				}
				getFile(sockfd, buffer + 4);
				continue;
			}
			else if (strncmp(buffer, "put", 3) == 0)
			{
				if (strlen(buffer) == 3 || strlen(buffer) == 4)
				{
					continue;
				}
				putFile(sockfd, buffer + 4);
				continue;
			}
			else if (strcmp(buffer, "quit") == 0 ||
				strcmp(buffer, "exit") == 0 ||
				strcmp(buffer, "end") == 0 ||
				strcmp(buffer, "bye") == 0 ||
				strcmp(buffer, "over") == 0)
			{
				exit(0);
			}
			else if (Identity == 1 && (strncmp(buffer, "close", 5) == 0))
			{
				send(sockfd, buffer, BUFFER_SIZE, 0);
				exit(0);
			}
            else if (strncmp(buffer, "help", 4) == 0)
            {
                if(Identity == 1)
                {
                    rootHelpInfo();
                }
                else
                {
                    userHelpInfo();
                }
            }
			else
			{
				if (Identity == 0 && (strncmp(buffer, "rm", 2) == 0 
					|| strncmp(buffer, "ps", 2) == 0))
				{
					*(buffer + 2) = '\0';
					printf("%s : Need to be root\n", buffer);
					continue;
				}
				int fd;
				if ((fd = open("command.txt", O_CREAT |
					O_WRONLY | O_TRUNC, 0664)) < 0)
				{
					printf("fail to open\n");
				}
				send(sockfd, buffer, strlen(buffer), 0);
				char readbuff[BUF_SIZE] = { 0 };
				ssize_t bytes;
				while ((bytes = recv(sockfd, readbuff, BUF_SIZE, 0)) > 0)
				{
					if (strncmp(readbuff, "over", 4) == 0)
					{
						break;
					}
					write(fd, readbuff, bytes);
				}
				close(fd);

				pid_t pid = fork();
				assert(pid != -1);

				int s_fd;
				if (pid == 0)
				{
					s_fd = open("command.txt", O_RDONLY, 0644);
					execl("./com.sh", "com.sh", (char*)0);
				}
				close(s_fd);
				wait(NULL);
			}
		}
	}
}

/*******************************************************************************   
 * Function:       create_to_ser() 
 * Description:    本函数的主要功能是进行与服务器端的连接，并将获取到的
 *                 套接字返回给功能主函数进行处理。
 * Calls:          NULL
 * Called By:      do_request()
 * Others:         系统主功能实现文件传输服务，因此采用可靠的面向连接的TCP
 *                 流式服务，确保文件完整传输给对方。
*******************************************************************************/
int create_to_ser()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	printf("sockfd = %d\n", sockfd);

	if (sockfd == -1)
	{
		return -1;
	}

	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(PORT);
	saddr.sin_addr.s_addr = inet_addr(IPSTR);

	int res = connect(sockfd, (struct sockaddr*) & saddr, sizeof(saddr));
	printf("res = %d\n", res);

	if (res == -1)
	{
		return -1;
	}

	return sockfd;
}


/*******************************************************************************   
 * Function:       getFile()
 * Description:    本函数实现上传文件的功能
 * Calls:          NULL
 * Called By:      do_request()
 * Table Accessed: md5table(DATABASE: md5),当该函数将文件名发送给服务器后，
 *                 服务器对应要在md5table表中查询该文件是否完整，若不完整，
 *                 则返回给客端错误码，不允许下载该文件。 
 * Table Updated:  NULL
 * Input:          socket - 连接套接字   filename - 待下载的文件名 
 * Output:         NULL   
 * Return:         NULL
 * Others:         本函数会在客端输出服务器发来的文件大小信息，下载完毕后
 *                 会再此计算本地文件大小，便于验证。
 *                 另外，本函数实现了大文件的下载、下载文件的断点续传、下载相同
 *                 文件的秒传功能，其中大文件下载要使用lseek64\ftruncate64等
 *                 函数实现。
*******************************************************************************/
void getFile(int sockfd, char* filename)
{
	char buffer[BUFFER_SIZE] = { 0 };
	int fd;
	long long bytes;
	if (filename[0] == ' ')
	{
		return;
	}

	sprintf(buffer, "get %s", filename);
	send(sockfd, buffer, BUFFER_SIZE, 0);

	recv(sockfd, buffer, BUFFER_SIZE, 0);

	if (strncmp(buffer, "NOT EXISTS", 10) == 0)
	{
		printf("The file don't exist！\n");
		return;
	}
	else if (strncmp(buffer, "ERROR", 5) == 0)
	{
		printf("%s is incomplete and cannot be downloaded.\n", filename);
		return;
	}

	printf("File Size(from server) : %s\n", buffer);
	long long Osize = atoll(buffer);
	sleep(1);

	int flag = 0;
	long long size = 0;
	if ((fd = open(filename, O_RDWR | O_LARGEFILE)) < 0)
	{
		if ((fd = open(filename, O_CREAT | O_WRONLY 
			| O_TRUNC | O_LARGEFILE, 0664)) < 0)
		{
			printf("fail to open\n");
		}
		flag = NEW_FILE;
	}
	else
	{
		size = lseek64(fd, 0, SEEK_END);
		if (Osize < size)
		{
			flag = NEW_FILE;
			ftruncate64(fd, 0);
			lseek64(fd, 0, SEEK_SET);
		}
		else if (Osize == size)
		{
			printf("The file has not changed since the last download.\n");
			flag = NOMOD_FILE;
		}
		else
		{
			flag = CON_FILE;
			printf("File Size(from local): %lld\n", size);
		}
	}

	char buf[BUF_SIZE] = { 0 };
	if (flag == NEW_FILE)
	{
		strcpy(buf, "OK");
		send(sockfd, buf, BUF_SIZE, 0);
		printf("%s is being downloaded, please wait a moment.\n", filename);
		while ((bytes = recv(sockfd, buf, BUF_SIZE, 0)) > 0)
		{
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

		send(sockfd, sizebuff, BUF_SIZE, 0);
		long long num = lseek64(fd, size, SEEK_SET);
		printf("Origin Size : %lld\n", num);
		num = 0;
		while ((bytes = recv(sockfd, buf, BUF_SIZE, 0)) > 0)
		{
			if (strncmp(buf, "##over##", 8) == 0)
			{
				break;
			}
			num += write(fd, buf, bytes);
		}
		printf("Continue Transport Size : %lld\n", num);
	}
	else if (flag == NOMOD_FILE)
	{
		strcpy(buf, "##over##");
		send(sockfd, buf, BUF_SIZE, 0);
		return;
	}

	close(fd);
	struct stat64 st;
	stat64(filename, &st);
	printf("%s : Download Over\n", filename);
	printf("File Size(from local) : %lld\n", st.st_size);
}


/*******************************************************************************   
 * Function:       putFile()
 * Description:    本函数实现下载文件的功能
 * Calls:          NULL
 * Called By:      do_request()
 * Table Accessed: md5table(DATABASE: md5),当该函数将计算待上传文件的文件名和
 *                 md5值传输给服务器，服务器需要进行相应操作，比如进行秒传功能的
 *                 实现，上传文件过程中服务器md5table中的该文件完整标志位为0，
 *                 当客端传送完毕时，服务器将相应的标志位置为1，表示文件完整。
 * Table Updated:  md5table(DATABASE: md5)
 * Input:          socket - 连接套接字   filename - 待上传的文件名 
 * Output:         NULL   
 * Return:         NULL
 * Others:         本函数除了基本的上传功能外，也实现了大文件的上传、上传文
 *                 件的断点续传、上传文件的秒传功能，其中大文件下载要使用open
 *                 函数的O_LARGEFILE标志位，lseek64\ftruncate64等函数实现。
 *                 文件大小都要使用long long 类型。
 *                 注意断点续传功能，必须是相同用户登录才能实现断点续传功能，
 *                 其他用户都是全部重传文件。这也符合实际的需求。
*******************************************************************************/
void putFile(int sockfd, char* filename)
{
	int fd;
	char buffer[BUFFER_SIZE] = {};
	long long bytes;
	if (filename[0] == ' ')
	{
		return;
	}
	if ((fd = open(filename, O_RDONLY | O_LARGEFILE)) < 0)
	{
		if (errno == ENOENT)
		{
			printf("The file don't exist！\n");
			return;
		}
		else
		{
			printf("fail to open");
		}
	}
	/* 发beigin使对方进入put函数 */
	sprintf(buffer, "put %s", filename);
	send(sockfd, buffer, BUFFER_SIZE, 0);

	/*收begin*/
	recv(sockfd, buffer, BUFFER_SIZE, 0);


	char md5res[BUFFER_SIZE] = { 0 };

	/* 向对方发送md5值 */
	pid_t pid = fork();
	assert(pid != -1);

	if (pid == 0)
	{
		execl("./main.sh", "main.sh", filename, (char*)0);
	}
	wait(NULL);
	int s_fd = open("md5.txt", O_RDONLY, 0644);
	read(s_fd, md5res, BUFFER_SIZE);
	send(sockfd, md5res, BUFFER_SIZE, 0);
	close(s_fd);

	recv(sockfd, buffer, BUFFER_SIZE, 0);
	if (strncmp(buffer, "exist", 5) == 0)
	{
		printf("Second transmission\n");
		strcpy(buffer, "over");
		send(sockfd, buffer, BUFFER_SIZE, 0);
		return;
	}

	strcpy(buffer, "begin");
	send(sockfd, buffer, BUFFER_SIZE, 0);

	recv(sockfd, buffer, BUFFER_SIZE, 0);

	//==========================================

	char tmp[BUF_SIZE] = { 0 };
	long long size = lseek64(fd, 0, SEEK_END);
	lseek64(fd, 0, SEEK_SET);

	sprintf(tmp, "%lld", size);

	char buf[BUF_SIZE] = { 0 };
	strcpy(buf, tmp);
	send(sockfd, buf, BUF_SIZE, 0);

	recv(sockfd, buf, BUF_SIZE, 0);
	if (strncmp(buf, "OK", 2) == 0)
	{

		while ((bytes = read(fd, buf, BUF_SIZE)) > 0)
		{
			send(sockfd, buf, bytes, 0);

		}
	}
	else
	{
		long long  index = 0;
		index = atoll(buf);
		long long num = lseek64(fd, index, SEEK_SET);
		printf("Origin Size : %lld\n", num);
		num = 0;
		while ((bytes = read(fd, buf, BUF_SIZE)) > 0)
		{
			num += bytes;
			send(sockfd, buf, bytes, 0);
		}
		printf("Continue Transport Size : %lld\n", num);
	}

	sleep(1);

	strcpy(buf, "##over##");
	send(sockfd, buf, BUF_SIZE, 0);

	printf("%s : Upload Over\n", filename);
}
