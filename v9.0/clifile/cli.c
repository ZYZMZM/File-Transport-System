#include <stdio.h>
#include <stdlib.h>
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
#include "user.h"

#define N 128
#define PORT 6000
#define IPSTR "192.168.204.129"
#define LIS_MAX 20

int create_to_ser();

void getFile(int sockfd, char* filename);

void putFile(int sockfd, char* filename);

int Identity = 0;

int main()
{
	int sockfd = create_to_ser();
	assert(sockfd != -1);
    
    int initFlag = 0;
    while (1)
    {
        if (initFlag == 0)
        {
            printf("请选择：0(登录)  1(注册)  ");
            char input[128] = { 0 };
            fgets(input, 128, stdin);
	        input[strlen(input) - 1] = 0;
            if (strncmp(input, "1", 1) == 0)
                initFlag = 1;
            else if (strncmp(input, "0", 1) == 0)
                initFlag = 2;
        }

        if (initFlag == 1)
        {
            printf("请输入用户名：");
	        char username[128] = { 0 };
            fgets(username, 128, stdin);
	        username[strlen(username) - 1] = 0;

            printf("请输入密  码：");
	        char password[128] = { 0 };
            fgets(password, 128, stdin);
	        password[strlen(password) - 1] = 0;
            
            int res = Register(username, password);
            if (res == 1)
            {
                printf("注册成功, 请登录\n");
                initFlag = 2;
            }
            else
            {
                initFlag = 1;
                continue;
            }
        }
        
        if (initFlag == 2)
        {
            printf("用户名：");
            char username[128] = { 0 };
            fgets(username, 128, stdin);
            username[strlen(username) - 1] = 0;

            printf("密  码：");
            char password[128] = { 0 };
            fgets(password, 128, stdin);
            password[strlen(password) - 1] = 0;

            int res = Match(username, password);
            if (res == 1)
            {
                printf("%s 登录成功\n", username);
                if (strcmp(username, "su") == 0)
                    Identity = 1;
                break;
            }
            else
            {
                printf("用户名或密码错误\n");
                initFlag = 2;
                continue;
            }
        }
    }

	while (1)
	{
		char buffer[128] = { 0 };
        char *str = "Connect>>";     
    
        char *p = readline(str);
		//fflush(stdout);
        
		//fgets(buffer, 128, stdin);
		//buffer[strlen(buffer) - 1] = 0;
        //printf("buffer = %s\n", buffer);
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
				if (strlen (buffer) == 3 || strlen (buffer) == 4)
				{continue;}
				getFile(sockfd, buffer + 4);
				continue;
			}
			else if (strncmp(buffer, "put", 3) == 0)
			{
				if (strlen (buffer) == 3 || strlen (buffer) == 4)
				{continue;}
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
            else if (Identity == 1 && (strncmp(buffer, "close", 5) == 0) )
            {
                send(sockfd, buffer, 128, 0);
                exit(0);
            }
			else
			{
                if (Identity == 0 && (strncmp(buffer, "rm", 2) == 0 || strncmp(buffer, "ps", 2) == 0) )
                {
                    *(buffer + 2) = '\0';
                    printf("%s : Need to be root\n", buffer);
                    continue;
                }
				int fd;
				if ((fd = open("command.txt", O_CREAT | O_WRONLY | O_TRUNC, 0664)) < 0)
				{
					printf("fail to open\n");
				}
				send(sockfd, buffer, strlen(buffer), 0);
				char readbuff[4096] = { 0 };
				ssize_t bytes;
				while ((bytes = recv(sockfd, readbuff, 4096, 0)) > 0)
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


void getFile(int sockfd, char* filename)
{
	char buffer[128] = { 0 };
	int fd;
	ssize_t bytes;
	if (filename[0] == ' ')
	{
		return;
	}

	sprintf(buffer, "get %s", filename);
	send(sockfd, buffer, 128, 0);

	printf("recv size = %d\n", recv(sockfd, buffer, 128, 0));

	if (strncmp(buffer, "NOT EXISTS", 10) == 0)
	{
		printf("文件不存在！\n");
		return;
	}	
   
    printf("Recv File Size:%s\n",buffer);
    int Osize = atoi(buffer);

	sleep(1);

	int flag = 0;
	int size = 0;
	if ((fd = open(filename, O_RDWR)) < 0)
	{
		if ((fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0664)) < 0)
		{
			printf("fail to open\n");
		}
		flag = 1;
	}
	else
	{
		size = lseek(fd, 0, SEEK_END);
		if(Osize < size)
		{
			flag = 1;
			ftruncate(fd, 0);
			lseek(fd, 0, SEEK_SET);
     		}
		else if(Osize == size)
   		{
			printf("下载完毕，文件自上次下载后，未有更改\n");
			flag = 3;
		}
		else
		{
			flag = 2;
			printf("Not Finish File Size = %d\n", size);
		}
	}

	if (flag == 1) {

		strcpy(buffer, "OK");
		send(sockfd, buffer, 128, 0);
		printf("正在下载%s,请稍候\n", filename);
		while ((bytes = recv(sockfd, buffer, 128, 0)) > 0)
		{
			if (strncmp(buffer, "over", 4) == 0)
			{
				break;
			}
			write(fd, buffer, bytes);
		}
	}

	else if (flag == 2)
	{
		char sizebuff[128] = { 0 };
		sprintf(sizebuff, "%d", size);
		send(sockfd, sizebuff, 128, 0);

		int num = lseek(fd, size, SEEK_SET);
		printf("Origin Size = %d\n", num);
		num = 0;
		while ((bytes = recv(sockfd, buffer, 128, 0)) > 0)
		{
			if (strncmp(buffer, "over", 4) == 0)
			{
				break;
			}
			num += write(fd, buffer, bytes);
		}
		printf("Continue Tran Size = %d\n", num);
	}
	else if (flag == 3)
	{
		strcpy(buffer, "over");
		send(sockfd, buffer, 128, 0);
		return;
	}

	size = lseek(fd, 0, SEEK_END);
	close(fd);
	printf("%s下载完毕，大小为%d\n", filename, size);
}

void putFile(int sockfd, char* filename)
{
	int fd;
	char buffer[128] = {};
	ssize_t bytes;
	if (filename[0] == ' ')
	{
		return;
	}
	if ((fd = open(filename, O_RDONLY)) < 0)
	{
		if (errno == ENOENT)
		{
			printf("文件不存在！\n");
			return;
		}
		else
		{
			printf("fail to open");
		}
	}
	sprintf(buffer, "put %s", filename);
	send(sockfd, buffer, 128, 0);


	recv(sockfd, buffer, 128, 0);
	printf("Buffer = %s\n", buffer);
	if (strncmp(buffer, "exist", 5) == 0)
	{
		printf("秒传完毕\n");
		strcpy(buffer, "over");
		send(sockfd, buffer, 128, 0);
		return;
	}

	strcpy(buffer, "begin");
	send(sockfd, buffer, 128, 0);

	recv(sockfd, buffer, 128, 0);
	printf("Recv Buffer = %s\n", buffer);

//==========================================

	char tmp[128] = {0};
	int size = lseek(fd,0,SEEK_END);
        lseek(fd,0,SEEK_SET);
  
        sprintf(tmp,"%d",size);
	strcpy(buffer, tmp);
	send(sockfd, buffer, 128, 0);
	
	recv(sockfd, buffer, 128, 0);
	if ((strncmp(buffer, "over", 4) == 0))
	{
		return;
	}
	else if (strncmp(buffer, "OK", 2) == 0)
	{
        sendfile(sockfd, fd, 0, size);
	}
	else
	{
		int index = 0;
		index = atoi(buffer);
		int num = lseek(fd, index, SEEK_SET);
		printf("Origin Size = %d\n", num);
		//lseek(fd, 0, SEEK_SET);
        bytes = sendfile(sockfd, fd, 0, size - num);
		printf("Continue Tran Size = %d\n", bytes);
	}

	sleep(1);

	strcpy(buffer, "over");
	send(sockfd, buffer, 128, 0);

	printf("send over\n");
}
