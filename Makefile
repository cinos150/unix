all: server 

server: server.c structures.h	
	gcc -Wall -o server server.c LinkedList.c message_flow.c Utilities.c -lpthread

.PHONY: clean
clean:
	-rm server 
