#include <stdio.h>
#include <string.h>
#include <openssl/md5.h> // 如果你直接拷贝我的程序运行， 那注定找不到md5.h
#pragma comment(lib, "libeay32.lib")
 
int main()
{
	MD5_CTX ctx;
	int len = 0;
	unsigned char buffer[1024] = {0};
	unsigned char digest[16] = {0};
	
	FILE *pFile = fopen ("master.mp4", "rb"); // 我没有判断空指针
	
	MD5_Init (&ctx);
 
	while ((len = fread (buffer, 1, 1024, pFile)) > 0)
	{
		MD5_Update (&ctx, buffer, len);
	}
 
	MD5_Final (digest, &ctx);
	
	fclose(pFile);
	
 
	int i = 0;
	char buf[33] = {0};
    char tmp[3] = {0};
    for(i = 0; i < 16; i++ )
	{
		sprintf(tmp,"%02X", digest[i]); // sprintf并不安全
		strcat(buf, tmp); // strcat亦不是什么好东西
    }
	
	printf("%s\n", buf); // 文件的md5值
 
    return 0;
}
