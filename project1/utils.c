#include "utils.h"
void invalid_format() {
  fprintf(stderr, "invalid or missing options\nusage: snc [­l] [­u] [hostname] port\n");
}

int is_numeric(char* input) {
  int len = strlen (input);
  for (int i = 0; i < len; i ++) {
    if (!isdigit(input[i])) {
      return 0;
    }
  }
  return 1;
}

int read_port(int argc, char* argv[], int* port) {
  char* last_entry = argv[argc-1];
  if (is_numeric(last_entry)) {
    int p = atoi(last_entry);
    if (p <= 1024 || p > 65535) {
      fprintf(stderr, "internal error\n");
      return -1;
    } else {
      *port = p;
      return 0;
    }
  } else {
    invalid_format();
    return -1;
  }
}

int server_read_and_print (int sock)
{
  int n;
  char buffer[256];
      
  bzero(buffer,256);
  n = read(sock, buffer, 255);

  if (strlen(buffer) == 0) {
    return 1;
  }          
  if (n < 0) {
    printf("here\n");
    printInternalError();
    return 1;
  }
  fprintf(stdout, "%s", buffer);
  return 0;
}

void printInternalError() {
  fprintf(stderr, "internal error\n");
  exit(1);
}

int hostname_to_ip(char * hostname , char* ip)
{
  struct hostent *he;
  struct in_addr **addr_list;
  int i;
    
  if ( (he = gethostbyname( hostname ) ) == NULL) 
  {
    // get the host info
    herror("gethostbyname");
    return 1;
  }

  addr_list = (struct in_addr **) he->h_addr_list;
  
  for(i = 0; addr_list[i] != NULL; i++) 
  {
    //Return the first one;
    strcpy(ip , inet_ntoa(*addr_list[i]) );
    return 0;
  }
  
  return 1;
}