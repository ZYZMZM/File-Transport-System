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
#include <sys/stat.h>
 
int main(int argc, char *argv[])
{
	struct stat64 statbuff;
	if( -1 == stat64(argv[1], &statbuff) )
	{
		printf("stat wrong.%ld\n", statbuff.st_size);
	}
	else
	{
        long long size = statbuff.st_size;
		printf("stat success.%lld\n",size);
	}
}

