all:cli

cli:cli.o printHelpInfo.o
	gcc -o cli cli.o printHelpInfo.o -I/usr/include -lreadline -ltermcap

cli.o:cli.c
	gcc -c cli.c -lreadline

printHelpInfo.o:printHelpInfo.c
	gcc -c printHelpInfo.c

clean:
	rm -rf *.o ser
