/************************************************************************
 * File name: printHelpInfo.c
 * Description: 本文件中封装了向客端打印帮助文件的API接口
 *  		    startHelpInfo() - 每个客端登录成功都会打印的启动信息
 *  		    rootHelpInfo() - root用户调用help命令时打印的帮助信息
 *  		    userHelpInfo() - 普通用户调用help命令时打印的帮助信息
 * Author: ZYZMZM
 * Date: 02/06/2019
 * Version: v12.0
 * History:
 * **********************************************************************/
#include "printHelpInfo.h"
#define BUFFER_SIZE 1024

void startHelpInfo(char *port)
{
    FILE * fp = fopen("startInfo.txt", "r");
    assert(fp != NULL);
    
    char buffer[BUFFER_SIZE] = {0};
    int num = 0;
    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {   
        ++num;
        if(num == 2)
        {
            printf("Your File Transport connection id is %s\n", port);
            continue;
        }
        printf("%s", buffer);
    }
    printf("\n");
    fclose(fp);
}

void rootHelpInfo()
{
    FILE * fp = fopen("rootHelpInfo.txt", "r");
    assert(fp != NULL);
    
    char buffer[BUFFER_SIZE] = {0};
    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        printf("%s", buffer);
    }
    printf("\n");
    fclose(fp);
}

void userHelpInfo()
{
    FILE * fp = fopen("userHelpInfo.txt", "r");
    assert(fp != NULL);
    
    char buffer[BUFFER_SIZE] = {0};
    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        printf("%s", buffer);
    }
    printf("\n");
    fclose(fp);
}
