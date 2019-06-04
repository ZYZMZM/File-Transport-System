/*********************************************************************************************
 * File name: user.c
 * Description: 本文件中封装了大量的服务器与数据库进行交互的API接口
 *  		    insertMd5Info() - 向文件信息表md5table中插入文件信息
 *  		    deleteMd5Info() - 在文件信息表md5table中删除指定文件信息
 *  		    initStart() - 该函数根据数据库serverLog中的启动次数判断文件信息表是否需要初始化
 *  		    Match() - 用户登录的用户名与密码的匹配
 *  		    Register() - 用户注册，包含判断用户名是否有效，成功后插入用户表user
 *  		    matchMd5() - 秒传功能的实现，根据客端发送的md5值与数据库中匹配
 *  		    setIntact() - 设置文件信息表中文件的完整标志位
 *  		    isVaildUer() - 判断用户是否是该文件的所属者
 *  		    isIntact() - 在文件信息表中根据完整标志位判断文件是否完整
 * Author: ZYZMZM
 * Date: 02/06/2019
 * Version: v12.0
 * History:
 * *********************************************************************************************/
#include "user.h"
#define SUCCESS 1
#define FAIL 0
#define ERROR -1
unsigned int timeout = 7;   //超时时间7秒


/******************************************************************************************
 * Function:       insertMd5Info()
 * Description:    向文件信息表md5table中插入文件信息
 * Calls:          NULL
 * Called By:      putFile() - thread.c
 * Table Accessed: md5table(DATABASE: md5)
 * Table Updated:  md5table(DATABASE: md5),此函数将文件的相关信息(文件名、文件md5值、所属
 *                 用户、完整程度)存入数据库中，完整标志以0存入 
 * Input:          filename - 要添加的文件名   md5value - 文件的md5值  
 *                 username - 该文件的所属用户
 * Return:         成功返回 1，由调用函数接收并判断
*******************************************************************************************/
int insertMd5Info(char* filename, char* md5value, char* username)
{
	MYSQL* mysql;
	MYSQL_RES* result;
	MYSQL_ROW row;
	int ret = 0;

	mysql = mysql_init(NULL);//初始化
	if (!mysql)
	{
		printf("mysql_init failed!\n");
		return ERROR;
	}

	ret = mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT,
		(const char*)& timeout);//设置超时选项

	if (ret)
	{
		printf("Options Set ERRO!\n");
	}

	mysql = mysql_real_connect(mysql, "localhost", "root",
		"123456", "md5", 0, NULL, 0);//连接MySQL testdb数据库

	if (mysql)
	{
		char buffer[128] = { 0 };
		sprintf(buffer, "select md5Value from md5table where filename='%s'", filename);

		ret = mysql_query(mysql, buffer);
		if (!ret)
		{
			result = mysql_store_result(mysql);
			int num = mysql_num_rows(result);
			if (num == 1)
			{
				deleteMd5Info(filename);
			}
			mysql_free_result(result);
		}

		char sql_insert[200];
		char* intact = "0";
		sprintf(sql_insert, "INSERT INTO md5table(filename,md5Value,username,intact) VALUES('%s','%s','%s', '%s');", filename, md5value, username, intact);

		ret = mysql_query(mysql, sql_insert); //执行SQL语句
		if (!ret)
		{
			return SUCCESS;
		}
		else
		{
			printf("Connect Erro:%d %s\n",
				mysql_errno(mysql), mysql_error(mysql));//返回错误代码、错误消息
			return ERROR;
		}

		mysql_close(mysql);
		printf("Connection closed!\n");
	}
	else    //错误处理
	{
		printf("Connection Failed!\n");
		if (mysql_errno(mysql))
		{
			printf("Connect Erro:%d %s\n",
				mysql_errno(mysql), mysql_error(mysql));//返回错误代码、错误消息
		}
		return ERROR;
	}

	return FAIL;
}



/******************************************************************************************
 * Function:       deleteMd5Info()
 * Description:    在文件信息表md5table中删除指定文件信息
 * Calls:          NULL
 * Called By:      work_thread() - thread.c
 * Table Accessed: md5table(DATABASE: md5)
 * Table Updated:  md5table(DATABASE: md5),由于root用户执行了rm命令，为了确保数据一致
 *                 性，因此调用此函数删除该文件信息
 * Input:          filename - 要删除的文件名
 * Return:         成功返回 1，由调用函数接收并判断
*******************************************************************************************/
int deleteMd5Info(char* filename)
{
	MYSQL* mysql;
	MYSQL_RES* result;
	MYSQL_ROW row;
	int ret = 0;

	mysql = mysql_init(NULL);//初始化
	if (!mysql)
	{
		printf("mysql_init failed!\n");
		return ERROR;
	}

	ret = mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT,
		(const char*)& timeout);//设置超时选项

	if (ret)
	{
		printf("Options Set ERRO!\n");
	}

	mysql = mysql_real_connect(mysql, "localhost",
		"root", "123456", "md5", 0, NULL, 0);//连接MySQL testdb数据库

	if (mysql)
	{
		if (!mysql_real_connect(mysql, "localhost", "root",
			"123456", "md5", 0, NULL, 0))
		{
			fprintf(stderr, "%d: %s  \n",
				mysql_errno(mysql), mysql_error(mysql));

			return FAIL;
		}
		//printf("连接成功\n");

		char buffer[128] = { 0 };
		sprintf(buffer, "select md5Value from md5table where filename='%s'", filename);

		ret = mysql_query(mysql, buffer);
		if (!ret)
		{
			result = mysql_store_result(mysql);
			int num = mysql_num_rows(result);
			if (num == 1)
			{
				sprintf(buffer, "delete from md5table where filename='%s'", filename);

				if (mysql_query(mysql, buffer))
				{
					fprintf(stderr, "%d: %s\n",
						mysql_errno(mysql), mysql_errno(mysql));
				}
				else
				{
					printf("删除成功\n");
					return SUCCESS;
				}

			}
			mysql_free_result(result);
		}


		mysql_close(mysql);
		return FAIL;
	}
}


/******************************************************************************************
 * Function:       initStart()
 * Description:    该函数根据数据库serverLog中的启动次数判断文件信息表是否需要初始化
 * Calls:          NULL
 * Called By:      main()
 * Table Accessed: serverStartInfo(DATABASE: serverLog)
 * Table Updated:  serverStartInfo(DATABASE: serverLog)，服务器每次启动都要执行相应脚本进行
 *                 启动日志的记录。该函数判断是否是首次启动，若是ser.c中调用其他脚本初始化
 *                 文件信息表md5table。
 * Input:          NULL
 * Return:         成功返回 1，由调用函数接收并判断
*******************************************************************************************/
int initStart()
{
	MYSQL* mysql;
	MYSQL_RES* result;
	MYSQL_ROW row;
	int ret = 0;

	mysql = mysql_init(NULL);//初始化
	if (!mysql)
	{
		printf("mysql_init failed!\n");
		return ERROR;
	}

	ret = mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT,
		(const char*)& timeout);//设置超时选项

	if (ret)
	{
		printf("Options Set ERRO!\n");
	}

	mysql = mysql_real_connect(mysql, "localhost",
		"root", "123456", "serverLog", 0, NULL, 0);//连接MySQL testdb数据库

	if (mysql)
	{
		if (!mysql_real_connect(mysql, "localhost", "root",
			"123456", "serverLog", 0, NULL, 0))
		{
			fprintf(stderr, "%d: %s  \n",
				mysql_errno(mysql), mysql_error(mysql));

			return FAIL;
		}

		char buffer[128] = { 0 };
		strcpy(buffer, "select * from serverStartInfo");

		if (mysql_query(mysql, buffer))
		{
			fprintf(stderr, "%d: %s\n",
				mysql_errno(mysql), mysql_errno(mysql));
		}
		else
		{
			result = mysql_store_result(mysql);
			int fields = mysql_num_fields(result);
			int count = 0;
			while (row = mysql_fetch_row(result))
			{
				++count;
			};
			if (count == 1)
			{
				printf("need Init\n");
				return SUCCESS;
			}
			else if (count > 1)
			{
				printf("Not Init\n");
				return FAIL;
			}

			mysql_free_result(result);
		}

		mysql_close(mysql);
		return FAIL;
	}
}


/******************************************************************************************
 * Function:       Match()
 * Description:    该函数负责用户登录的用户名与密码的匹配验证
 * Calls:          NULL
 * Called By:      work_thread() - thread.c
 * Table Accessed: user(DATABASE: loginUser)
 * Table Updated:  user(DATABASE: loginUser)
 * Input:          username - 客端输入的用户名   passeword - 客端输入的登录密码
 * Return:         成功返回 1，由调用函数接收并判断
*******************************************************************************************/
int Match(char* username, char* password)
{
	MYSQL* mysql;
	MYSQL_RES* result;
	MYSQL_ROW row;
	int ret = 0;

	mysql = mysql_init(NULL);//初始化
	if (!mysql)
	{
		printf("mysql_init failed!\n");
		return ERROR;
	}

	ret = mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT,
		(const char*)& timeout);//设置超时选项

	if (ret)
	{
		printf("Options Set ERRO!\n");
	}

	mysql = mysql_real_connect(mysql, "localhost",
		"root", "123456", "loginUser", 0, NULL, 0);//连接MySQL testdb数据库

	if (mysql)
	{
		if (!mysql_real_connect(mysql, "localhost", "root",
			"123456", "loginUser", 0, NULL, 0))
		{
			fprintf(stderr, "%d: %s  \n",
				mysql_errno(mysql), mysql_error(mysql));

			return FAIL;
		}
		//printf("连接成功\n");

		char buffer[128] = { 0 };
		sprintf(buffer, "select passwd from user where name='%s'", username);

		if (mysql_query(mysql, buffer))
		{
			fprintf(stderr, "%d: %s\n",
				mysql_errno(mysql), mysql_errno(mysql));
		}
		else
		{
			result = mysql_store_result(mysql);
			while (row = mysql_fetch_row(result))
			{
				// printf("\%s - \%s \n", row[0], row[1]);
				if (strcmp(row[0], password) == 0)
				{
					mysql_free_result(result);
					mysql_close(mysql);
					return SUCCESS;
				}
			}
			mysql_free_result(result);
		}

		mysql_close(mysql);
		return FAIL;
	}
}

/******************************************************************************************
 * Function:       Register()
 * Description:    用户注册，包含判断用户名是否有效，成功后插入用户表user
 * Calls:          NULL
 * Called By:      work_thread() - thread.c
 * Table Accessed: user(DATABASE: loginUser)
 * Table Updated:  user(DATABASE: loginUser)
 * Input:          username - 客端输入的注册用户名   passeword - 客端注册的登录密码
 * Return:         成功返回 1，由调用函数接收并判断
******************************************************************************************/
int Register(char* username, char* password)
{
	int ret = 0;
	MYSQL* mysql;
	MYSQL_RES* result;
	MYSQL_ROW row;

	mysql = mysql_init(NULL);//初始化
	if (!mysql)
	{
		printf("mysql_init failed!\n");
		return ERROR;
	}

	ret = mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT,
		(const char*)& timeout);//设置超时选项

	if (ret)
	{
		printf("Options Set ERRO!\n");
	}

	mysql = mysql_real_connect(mysql, "localhost", "root",
		"123456", "loginUser", 0, NULL, 0);//连接MySQL testdb数据库

	if (mysql)
	{
		//printf("Connection Succeed!\n");

		char buffer[128] = { 0 };
		sprintf(buffer, "select passwd from user where name='%s'", username);

		ret = mysql_query(mysql, buffer);
		// printf("ret = %d\n", ret);
		if (!ret)
		{
			result = mysql_store_result(mysql);
			int num = mysql_num_rows(result);
			//printf("judge num = %d\n", num);
			if (num == 1)
			{
				printf("%s已存在，请更换用户名\n", username);
				return 2;
			}
			mysql_free_result(result);
		}

		char sql_insert[200];
		sprintf(sql_insert, "INSERT INTO user(name,passwd) VALUES('%s','%s');", username, password);

		ret = mysql_query(mysql, sql_insert); //执行SQL语句
		if (!ret)
		{
			return SUCCESS;
		}
		else
		{
			printf("Connect Erro:%d %s\n",
				mysql_errno(mysql), mysql_error(mysql));//返回错误代码、错误消息
			return ERROR;
		}

		mysql_close(mysql);
		printf("Connection closed!\n");
	}
	else    //错误处理
	{
		printf("Connection Failed!\n");
		if (mysql_errno(mysql))
		{
			printf("Connect Erro:%d %s\n",
				mysql_errno(mysql), mysql_error(mysql));//返回错误代码、错误消息
		}
		return ERROR;
	}

	return FAIL;
}

int Display()
{
	MYSQL* mysql;
	MYSQL_RES* result;
	MYSQL_ROW row;
	int ret = 0;

	mysql = mysql_init(NULL);//初始化
	if (!mysql)
	{
		printf("mysql_init failed!\n");
		return -1;
	}

	ret = mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT,
		(const char*)& timeout);//设置超时选项

	if (ret)
	{
		printf("Options Set ERRO!\n");
	}

	mysql = mysql_real_connect(mysql, "localhost",
		"root", "123456", "loginUser", 0, NULL, 0);//连接MySQL testdb数据库

	if (mysql)
	{
		if (!mysql_real_connect(mysql, "localhost", "root",
			"123456", "loginUser", 0, NULL, 0))
		{
			fprintf(stderr, "%d: %s  \n",
				mysql_errno(mysql), mysql_error(mysql));

			return FAIL;
		}
		printf("连接成功\n");


		if (mysql_query(mysql, "select name, passwd from user where name != 'su'"))
		{
			fprintf(stderr, "%d: %s\n",
				mysql_errno(mysql), mysql_errno(mysql));
		}
		else
		{
			result = mysql_store_result(mysql);

			printf("=======用户信息表=======\n");
			printf("用户名 \t\t 密码\n");
			while (row = mysql_fetch_row(result))
			{
				printf("\%-10s \t \%-10s \n", row[0], row[1]);
			}
			return SUCCESS;
			mysql_free_result(result);
		}

		mysql_close(mysql);
		return FAIL;
	}
}


/******************************************************************************************
 * Function:       matchMd5()
 * Description:    秒传功能的实现，根据客端发送的md5值与数据库中匹配
 * Calls:          NULL
 * Called By:      putFile() - thread.c
 * Table Accessed: md5table(DATABASE: md5)
 * Table Updated:  md5table(DATABASE: md5)
 * Input:          filename - 客端上传的文件名   md5value - 由客端计算并发送给服务器的md5值
 * Return:         成功返回 1，由调用函数接收并判断
******************************************************************************************/
int matchMd5(char* filename, char* md5value)
{
	MYSQL* mysql;
	MYSQL_RES* result;
	MYSQL_ROW row;
	int ret = 0;

	mysql = mysql_init(NULL);//初始化
	if (!mysql)
	{
		printf("mysql_init failed!\n");
		return ERROR;
	}

	ret = mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT,
		(const char*)& timeout);//设置超时选项

	if (ret)
	{
		printf("Options Set ERRO!\n");
	}

	mysql = mysql_real_connect(mysql, "localhost",
		"root", "123456", "md5", 0, NULL, 0);//连接MySQL testdb数据库

	if (mysql)
	{
		if (!mysql_real_connect(mysql, "localhost", "root",
			"123456", "md5", 0, NULL, 0))
		{
			fprintf(stderr, "%d: %s  \n",
				mysql_errno(mysql), mysql_error(mysql));

			return FAIL;
		}
		//printf("连接成功\n");

		char buffer[128] = { 0 };
		sprintf(buffer, "select md5Value from md5table where filename='%s'", filename);

		if (mysql_query(mysql, buffer))
		{
			fprintf(stderr, "%d: %s\n",
				mysql_errno(mysql), mysql_errno(mysql));
		}
		else
		{
			result = mysql_store_result(mysql);
			while (row = mysql_fetch_row(result))
			{
				// printf("\%s - \%s \n", row[0], row[1]);
				if (strcmp(row[0], md5value) == 0)
				{
					mysql_free_result(result);
					mysql_close(mysql);
					return SUCCESS;
				}
			}
			mysql_free_result(result);
		}

		mysql_close(mysql);
		return FAIL;
	}
}

/******************************************************************************************
 * Function:       setIntact()
 * Description:    设置文件信息表中文件的完整标志位
 * Calls:          NULL
 * Called By:      putFile() - thread.c
 * Table Accessed: md5table(DATABASE: md5)
 * Table Updated:  md5table(DATABASE: md5)，客端上传文件前调用insertMd5Info()函数插入文件
 *                 信息，设置完整标志位为0, 若客端上传完毕，服务器便调用该函数将该文件的
 *                 完整标志位置为 1
 * Input:          filename - 客端上传的文件名   intact - 设置文件的完整标志位
 * Return:         成功返回 1，由调用函数接收并判断
******************************************************************************************/
int setIntact(char* filename, char* intact)
{
	int ret = 0;
	MYSQL* mysql;
	MYSQL_RES* result;
	MYSQL_ROW row;

	mysql = mysql_init(NULL);//初始化
	if (!mysql)
	{
		printf("mysql_init failed!\n");
		return ERROR;
	}

	ret = mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT,
		(const char*)& timeout);//设置超时选项

	if (ret)
	{
		printf("Options Set ERRO!\n");
	}

	mysql = mysql_real_connect(mysql, "localhost", "root",
		"123456", "md5", 0, NULL, 0);//连接MySQL testdb数据库

	if (mysql)
	{
		char sql_alter[200];
		sprintf(sql_alter, "UPDATE md5table SET intact='%s' where filename='%s';", intact, filename);

		ret = mysql_query(mysql, sql_alter); //执行SQL语句
		if (!ret)
		{
			return SUCCESS;
		}
		else
		{
			printf("Connect Erro:%d %s\n",
				mysql_errno(mysql), mysql_error(mysql));//返回错误代码、错误消息
			return ERROR;
		}

		mysql_close(mysql);
		printf("Connection closed!\n");
	}
	else    //错误处理
	{
		printf("Connection Failed!\n");
		if (mysql_errno(mysql))
		{
			printf("Connect Erro:%d %s\n",
				mysql_errno(mysql), mysql_error(mysql));//返回错误代码、错误消息
		}
		return ERROR;
	}

	return FAIL;
}


/******************************************************************************************
 * Function:       isVaildUer()
 * Description:    判断用户是否是该文件的所属者
 * Calls:          NULL
 * Called By:      putFile() - thread.c
 * Table Accessed: md5table(DATABASE: md5)
 * Table Updated:  md5table(DATABASE: md5)，断点续传前首先要进行文件所属者的判断，若该文件
 *                 不是当前用户上传的，那么当前用户不能续传，只能重传。
 * Input:          filename - 客端上传的文件名   username - 当前登录的用户名
 * Return:         成功返回 1，由调用函数接收并判断
******************************************************************************************/
int isVaildUser(char* filename, char* username)
{
	MYSQL* mysql;
	MYSQL_RES* result;
	MYSQL_ROW row;
	int ret = 0;

	mysql = mysql_init(NULL);//初始化
	if (!mysql)
	{
		printf("mysql_init failed!\n");
		return ERROR;
	}

	ret = mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT,
		(const char*)& timeout);//设置超时选项

	if (ret)
	{
		printf("Options Set ERRO!\n");
	}

	mysql = mysql_real_connect(mysql, "localhost",
		"root", "123456", "md5", 0, NULL, 0);//连接MySQL testdb数据库

	if (mysql)
	{
		if (!mysql_real_connect(mysql, "localhost", "root",
			"123456", "md5", 0, NULL, 0))
		{
			fprintf(stderr, "%d: %s  \n",
				mysql_errno(mysql), mysql_error(mysql));

			return FAIL;
		}
		//printf("连接成功\n");

		char buffer[128] = { 0 };
		sprintf(buffer, "select username from md5table where filename='%s'", filename);

		if (mysql_query(mysql, buffer))
		{
			fprintf(stderr, "%d: %s\n",
				mysql_errno(mysql), mysql_errno(mysql));
		}
		else
		{
			result = mysql_store_result(mysql);
			while (row = mysql_fetch_row(result))
			{
				// printf("\%s - \%s \n", row[0], row[1]);
				if (strcmp(row[0], username) == 0)
				{
					mysql_free_result(result);
					mysql_close(mysql);
					return SUCCESS;
				}
			}
			mysql_free_result(result);
		}

		mysql_close(mysql);
		return FAIL;
	}
}

/******************************************************************************************
 * Function:       isIntact()
 * Description:    在文件信息表中根据完整标志位判断文件是否完整
 * Calls:          NULL
 * Called By:      putFile()、getFile() - thread.c
 * Table Accessed: md5table(DATABASE: md5)
 * Table Updated:  md5table(DATABASE: md5)，是否进行断点续传的首要判断该文件标志位是否完整，
 *                 另外，客户端下载文件时，服务器也要根据该函数判断客户端待下载的文件是否
 *                 完整，若不完整则不允许客端下载
 * Input:          filename - 需要判断的文件名
 * Return:         成功返回 1，由调用函数接收并判断
******************************************************************************************/
int isIntact(char* filename)
{
	MYSQL* mysql;
	MYSQL_RES* result;
	MYSQL_ROW row;
	int ret = 0;

	mysql = mysql_init(NULL);//初始化
	if (!mysql)
	{
		printf("mysql_init failed!\n");
		return ERROR;
	}

	ret = mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT,
		(const char*)& timeout);//设置超时选项

	if (ret)
	{
		printf("Options Set ERRO!\n");
	}

	mysql = mysql_real_connect(mysql, "localhost",
		"root", "123456", "md5", 0, NULL, 0);//连接MySQL testdb数据库

	if (mysql)
	{
		if (!mysql_real_connect(mysql, "localhost", "root",
			"123456", "md5", 0, NULL, 0))
		{
			fprintf(stderr, "%d: %s  \n",
				mysql_errno(mysql), mysql_error(mysql));

			return FAIL;
		}
		//printf("连接成功\n");

		char buffer[128] = { 0 };
		sprintf(buffer, "select intact from md5table where filename='%s'", filename);

		if (mysql_query(mysql, buffer))
		{
			fprintf(stderr, "%d: %s\n",
				mysql_errno(mysql), mysql_errno(mysql));
		}
		else
		{
			result = mysql_store_result(mysql);
			while (row = mysql_fetch_row(result))
			{
				// printf("\%s - \%s \n", row[0], row[1]);
				if (strcmp(row[0], "1") == 0)
				{
					mysql_free_result(result);
					mysql_close(mysql);
					return SUCCESS;
				}
			}
			mysql_free_result(result);
		}

		mysql_close(mysql);
		return FAIL;
	}
}
