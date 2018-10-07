#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void invalid_format();

int is_numeric(char* input);

int read_port(int argc, char* argv[], int* port);

void printInternalError();