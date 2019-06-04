all:ser

ser:ser.o socket.o thread.o 
	gcc -o ser ser.o socket.o thread.o -pthread 

ser.o:ser.c
	gcc -c ser.c

socket.o:socket.c
	gcc -c socket.c

thread.o:thread.c
	gcc -c thread.c

clean:
	rm -rf *.o ser
