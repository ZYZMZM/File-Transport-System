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
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int fd = open(argv[1], O_RDONLY | O_LARGEFILE);
    printf("fd = %d\n", fd);
    
    int  size = lseek64(fd, 0, SEEK_END);
    printf("size = %lld\n", size);
    lseek64(fd, 0, SEEK_SET);
    close(fd);
    exit(0);
}
