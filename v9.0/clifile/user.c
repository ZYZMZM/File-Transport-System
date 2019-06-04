#include "user.h"

unsigned int timeout = 7;   //超时时间7秒

int Match(char *username, char *password)
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
    "root","123456","loginUser",0,NULL,0);//连接MySQL testdb数据库
    
    if(mysql)
    {
        if(!mysql_real_connect(mysql, "localhost", "root", 
            "123456", "loginUser", 0, NULL, 0))
        {
            fprintf(stderr, "%d: %s  \n", 
            mysql_errno(mysql), mysql_error(mysql));
            
            return 0;
        }
        //printf("连接成功\n");
        
        char buffer[128] = {0};
        sprintf(buffer, "select passwd from user where name='%s'", username);
        
        if(mysql_query(mysql, buffer))
        {
            fprintf(stderr, "%d: %s\n", 
            mysql_errno(mysql), mysql_errno(mysql));
        }
        else
        {
            result = mysql_store_result(mysql);
            while(row = mysql_fetch_row(result))
            {
               // printf("\%s - \%s \n", row[0], row[1]);
                if(strcmp(row[0], password)  == 0)
                {
                    mysql_free_result(result);
                    mysql_close(mysql);
                    return 1;
                }
            }
            mysql_free_result(result);
        }

        mysql_close(mysql);
        return 0;
    }
}

int Register(char *username, char *password)
{
    int ret = 0;
    MYSQL *mysql;
    MYSQL_RES *result;
    MYSQL_ROW row;
    
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
    "123456", "loginUser",0,NULL,0);//连接MySQL testdb数据库
    
    if(mysql)
    {
        //printf("Connection Succeed!\n");
        
        char buffer[128] = {0};
        sprintf(buffer, "select passwd from user where name='%s'", username);
        
        ret = mysql_query(mysql, buffer);
       // printf("ret = %d\n", ret);
        if(!ret)
        {
            result = mysql_store_result(mysql);
            int num = mysql_num_rows(result);
            //printf("judge num = %d\n", num);
            if(num == 1)
            {
                printf("%s已存在，请更换用户名\n", username);
                return 2;
            }
            mysql_free_result(result);
        }
	 
	    char sql_insert[200];
        sprintf(sql_insert, "INSERT INTO user(name,passwd) VALUES('%s','%s');", username, password);
	
        ret = mysql_query(mysql, sql_insert); //执行SQL语句
        if(!ret)
        {
            //printf("num = %d\n", (unsigned long)mysql_affected_rows(mysql));
            //printf("Inserted %lu rows\n",
            //(unsigned long)mysql_affected_rows(mysql));//返回上次UPDATE更改行数
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

int Display()
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
    "root","123456","loginUser",0,NULL,0);//连接MySQL testdb数据库
    
    if(mysql)
    {
        if(!mysql_real_connect(mysql, "localhost", "root", 
            "123456", "loginUser", 0, NULL, 0))
        {
            fprintf(stderr, "%d: %s  \n", 
            mysql_errno(mysql), mysql_error(mysql));
            
            return 0;
        }
        printf("连接成功\n");
        
        
        if(mysql_query(mysql, "select name, passwd from user where name != 'su'"))
        {
            fprintf(stderr, "%d: %s\n", 
            mysql_errno(mysql), mysql_errno(mysql));
        }
        else
        {
            result = mysql_store_result(mysql);

            printf("=======用户信息表=======\n");
            printf("用户名 \t\t 密码\n");
            while(row = mysql_fetch_row(result))
            {
               printf("\%-10s \t \%-10s \n", row[0], row[1]);
            }
            return 1;
            mysql_free_result(result);
        }

        mysql_close(mysql);
        return 0;
    }
}
