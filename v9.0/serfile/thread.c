#define _GNU_SOURCE 1
#include "thread.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#define ARGC 10
#define READ_BUFF 4096
#define N 128
void* work_thread(void* arg);
int thread_start(int connfd)
{
    pthread_t id; // 定义线程号
    /* 创建新线程，将连接套接字connfd作为参数传入 */
    int res = pthread_create(&id, NULL, work_thread, (void*)connfd);
    if (res != 0)
    {
        return -1;
    }
    return 0;
}

void getFile(int connfd, char* filename);
void putFile(int connfd, char* filename);

void* work_thread(void* arg)
{
    int connfd = (int)arg;


    while (1)
    {
        char buffer[256] = { 0 };

        int n = recv(connfd, buffer, 255, 0);
        
        if (strncmp(buffer, "close", 5) == 0)
        {
            printf("\nThe Server is closing\n");
            exit(0);
        }
        //printf("%d\n", n);
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
            getFile(connfd, myargv[1]);
            continue;
        }

        else if (strcmp(cmd, "put") == 0)
        {
            //上传
            putFile(connfd, myargv[1]);
            continue;
        }

        else
        {
            printf("cmd == %s\n", cmd);
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
        printf("buffer(%d) = %s\n", n, buffer);
    }
    printf("One client over!\n");
    /* close关闭套接字 */
    close(connfd);
}



void getFile(int connfd, char* filename)
{
    int fd;
    char buffer[128] = {};

    ssize_t bytes;
    if ((fd = open(filename, O_RDONLY)) < 0)
    {
        if (errno == ENOENT)
        {
            strcpy(buffer, "NOT EXISTS");
            send(connfd, buffer, 128, 0);
            return;
        }
        else
        {
            printf("fail to open");
        }
    }

    char tmp[128] = {0};
    int size = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);

    sprintf(tmp,"%d",size);
    strcpy(buffer, tmp);
    printf("send : %d\n", send(connfd, buffer, 128, 0));
    printf("send size = %s\n", buffer);

    sleep(1);

    struct pollfd fds[1];
    fds[0].fd = connfd;
    fds[0].events = POLLRDHUP;
    fds[0].revents = 0;


    recv(connfd, buffer, 128, 0);
    if ((strncmp(buffer, "over", 4) == 0))
    {
        return;
    }
    else if (strncmp(buffer, "OK", 2) == 0)
    {
        while ( 1 )
        {
            int n = poll(fds, 1, 0);
            if(n < 0)
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
            if ( sendfile(connfd, fd, 0, size) == 0)
                break;
        }
    }
    else
    {
        int index = 0;
        index = atoi(buffer);
        int num = lseek(fd, index, SEEK_SET);
        printf("num = %d\n", num);
        num = 0;
        while ( 1 )
        {
            int n = poll(fds, 1, 0);
            if(n < 0)
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
            if ( sendfile(connfd, fd, 0, size) == 0)
                break;
            //send(connfd, buffer, bytes, MSG_NOSIGNAL);
            //send(connfd, buffer, bytes, 0);
        }
        printf("num = %d\n", num);
    }

    sleep(1);

    strcpy(buffer, "over");
    //send(connfd, buffer, 128, MSG_NOSIGNAL);
    send(connfd, buffer, 128, 0);

    printf("send over\n");
}

void putFile(int connfd, char* filename)
{
    char buffer[128] = { 0 };
    int fd;
    ssize_t bytes;
    int flag = 0;


    pid_t pid = fork();
    assert(pid != -1);

    int s_fd;
    if (pid == 0)
    {
        s_fd = open("res.txt", O_CREAT | O_RDWR, 0644);
        dup2(s_fd, 1);
        dup2(s_fd, 2);
        execl("./main.sh", "main.sh", filename, (char*)0);
    }
    close(s_fd);
    wait(NULL);

    s_fd = open("res.txt", O_RDONLY, 0644);
    char res[128] = { 0 };
    if ((read(s_fd, res, 128)) > 0)
    {
        if (strncmp(res, "1", 1) == 0)
        {
            strcpy(buffer, "exist");
            send(connfd, buffer, 128, 0);
            recv(connfd, buffer, 128, 0);
            if (strncmp(buffer, "over", 4) == 0)
            {
                close(s_fd);
                return;
            }
        }
        else
        {
            strcpy(buffer, "no");
            send(connfd, buffer, 128, 0);
            recv(connfd, buffer, 128, 0);
        }

    }
    close(s_fd);

    strcpy(buffer, "begin");
    send(connfd, buffer, 128, 0);
    printf("begin\n");

    //=========================

    recv(connfd, buffer, 128, 0);
    printf("Recv File Size:%s\n",buffer);
    int Osize = atoi(buffer);

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
        else if (Osize == size)
        {
            printf("下载完毕，文件自上次下载后，未有更改\n");
            flag = 3;
        }
        else if (Osize > size)
        {
            flag = 2;
            printf("Not Finish File Size = %d\n", size);
        }
    }
    
    struct pollfd fds[1];
    fds[0].fd = connfd;
    fds[0].events = POLLRDHUP;
    fds[0].revents = 0;
    
    if (flag == 1) 
    {
        strcpy(buffer, "OK");
        send(connfd, buffer, 128, 0);
        printf("正在下载%s,请稍候\n", filename);
        while ((bytes = recv(connfd, buffer, 128, 0)) > 0)
        {
            int n = poll(fds, 1, 0);
            if(n < 0)
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
        send(connfd, sizebuff, 128, 0);

        int num = lseek(fd, size, SEEK_SET);
        printf("Origin Size = %d\n", num);
        num = 0;
        while ((bytes = recv(connfd, buffer, 128, 0)) > 0)
        {
            int n = poll(fds, 1, 0);
            if(n < 0)
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
        send(connfd, buffer, 128, 0);
        return;
    }
    size = lseek(fd, 0, SEEK_END);
    close(fd);
    printf("%s下载完毕，大小为%d\n", filename, size);
}
