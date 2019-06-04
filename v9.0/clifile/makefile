all:cli

cli:cli.o user.o
	gcc -o cli cli.o user.o `mysql_config --libs` -I/usr/include -lreadline -ltermcap

cli.o:cli.c
	gcc -c cli.c -lreadline

user.o:user.c
	gcc -c `mysql_config --cflags` user.c

clean:
	rm -rf *.o ser
