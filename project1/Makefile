snc : server.o client.o utils.o snc.c
	gcc -o snc utils.o client.o server.o snc.c

server.o : server.h server.c
	gcc -c server.h server.c

client.o : client.h client.c
	gcc -c client.h client.c

utils.o : utils.h utils.c
	gcc -c utils.c utils.h

clean: 
	rm *.o