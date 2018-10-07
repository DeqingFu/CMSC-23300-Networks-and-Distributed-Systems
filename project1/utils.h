#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

void invalid_format();

int is_numeric(char* input);

int read_port(int argc, char* argv[], int* port);

int server_read_and_print (int sock, char flag);

void printInternalError();

int hostname_to_ip(char * hostname , char* ip);


