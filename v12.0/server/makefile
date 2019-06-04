all:ser

ser:ser.o socket.o thread.o user.o
	gcc -o ser ser.o socket.o thread.o user.o -pthread `mysql_config --libs` -I/usr/include -lreadline -ltermcap

ser.o:ser.c
	gcc -c ser.c

socket.o:socket.c
	gcc -c socket.c

thread.o:thread.c
	gcc -c thread.c

user.o:user.c
	gcc -c `mysql_config --cflags` user.c

clean:
	rm -rf *.o ser
