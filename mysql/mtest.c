#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

unsigned int timeout = 7;   //超时时间7秒

int insertMd5Info(char *filename, char *md5value, char *username)
{
    MYSQL *mysql;
    MYSQL_RES *result;
    MYSQL_ROW row;
    int ret = 0;
    
    mysql = mysql_init(NULL);//初始化
    if(!mysql)
    {
        printf("mysql_init failed!\n");
        return -1;
    }

    ret = mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, 
    (const char*)&timeout);//设置超时选项
    
    if(ret)
    {
        printf("Options Set ERRO!\n");
    }
    
    mysql = mysql_real_connect(mysql, "localhost", "root", 
    "123456", "md5",0,NULL,0);//连接MySQL testdb数据库
    
    if(mysql)
    {
        char buffer[128] = {0};
        sprintf(buffer, "select md5Value from md5table where filename='%s'", filename);
        
        ret = mysql_query(mysql, buffer);
        if(!ret)
        {
            result = mysql_store_result(mysql);
            int num = mysql_num_rows(result);
            if(num == 1)
            {
                deleteMd5Info(filename);
            }
            mysql_free_result(result);
        }
	 
	char sql_insert[200];
	char *intact = "0";
        sprintf(sql_insert, "INSERT INTO md5table(filename,md5Value,username,intact) VALUES('%s','%s','%s', '%s');", filename,md5value,username,intact);
	
        ret = mysql_query(mysql, sql_insert); //执行SQL语句
        if(!ret)
        {
            return 1;
        }
        else
        {
            printf("Connect Erro:%d %s\n",
            mysql_errno(mysql),mysql_error(mysql));//返回错误代码、错误消息
            return -1;
        }

        mysql_close(mysql);
        printf("Connection closed!\n");
    }
    else    //错误处理
    {
        printf("Connection Failed!\n");
        if(mysql_errno(mysql))
        {
            printf("Connect Erro:%d %s\n",
            mysql_errno(mysql),mysql_error(mysql));//返回错误代码、错误消息
        }
        return -2;
    }

    return 0;
}

int deleteMd5Info(char *filename)
{
    MYSQL *mysql;
    MYSQL_RES *result;
    MYSQL_ROW row;
    int ret = 0;

    mysql = mysql_init(NULL);//初始化
    if(!mysql)
    {
        printf("mysql_init failed!\n");
        return -1;
    }

    ret = mysql_options(mysql,MYSQL_OPT_CONNECT_TIMEOUT,
    (const char*)&timeout);//设置超时选项
    
    if(ret)
    {
        printf("Options Set ERRO!\n");
    }
    
    mysql = mysql_real_connect(mysql, "localhost",
    "root","123456","md5",0,NULL,0);//连接MySQL testdb数据库
    
    if(mysql)
    {
        if(!mysql_real_connect(mysql, "localhost", "root", 
            "123456", "md5", 0, NULL, 0))
        {
            fprintf(stderr, "%d: %s  \n", 
            mysql_errno(mysql), mysql_error(mysql));
            
            return 0;
        }
        //printf("连接成功\n");
        
        char buffer[128] = {0};
        sprintf(buffer, "select md5Value from md5table where filename='%s'", filename);
        
        ret = mysql_query(mysql, buffer);
        if(!ret)
        {
            result = mysql_store_result(mysql);
            int num = mysql_num_rows(result);
            if(num == 1)
            {
              	 sprintf(buffer, "delete from md5table where filename='%s'", filename);
        
		if(mysql_query(mysql, buffer))
		{
		    fprintf(stderr, "%d: %s\n", 
		    mysql_errno(mysql), mysql_errno(mysql));
		}
		else
		{
				printf("删除成功\n");
				return 1;
		}

 	    }
		    mysql_free_result(result);
	  }

       
        mysql_close(mysql);
        return 0;
    }
}
int main()
{

insertMd5Info("cli.c","zyzmzmmzmzmzmzmz","zy");
    
}
