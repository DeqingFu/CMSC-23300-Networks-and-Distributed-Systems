#include "utils.h"
void invalid_format() {
  fprintf(stderr, "invalid or missing options\nusage: snc [-l] [-u] [hostname] port\n");
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

void TCP_server(int port, char* hostname) {
  int sockfd, newsockfd;
  unsigned int clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  int socket_option = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &socket_option, sizeof(socket_option));
  if (sockfd < 0) {
    printInternalError();
  } 

  bzero((char*) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);
  
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    printInternalError();
  }
  listen(sockfd, 5);
  clilen = sizeof(cli_addr);
  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  if (newsockfd < 0) {
    printInternalError();
  }
  while (1) {
    //fprintf(stderr, "%s\n", cli_addr.sin_addr);
    if (strlen(hostname) != 0) {
      char target_ip[128];
      hostname_to_ip(hostname, target_ip);
      //fprintf(stderr, "target ip: %s\n", target_ip);
      char client_ip[128];
      strcpy(client_ip, (char*)inet_ntoa((struct in_addr)cli_addr.sin_addr));
      //fprintf(stderr, "client ip: %s\n", client_ip);
      if (strcmp(client_ip, target_ip)) {
        continue;
      }
    }

    if (server_read_and_print(newsockfd)) {
      close(newsockfd);
      break;
    }
    
  }
  close(newsockfd);
}


void TCP_client(int port, char* hostname) {
  int sockfd, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[256];
  /* Create a socket point */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  int option = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  if (sockfd < 0) {
    printInternalError();
  }
  server = gethostbyname(hostname);
   
  if (server == NULL) {
    printf("here\n");
    printInternalError();
  }
   
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(port);
  /* Now connect to the server */
  if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    printInternalError();
  }
  while (1) {
    bzero(buffer,256);
    char* ret = fgets(buffer,255,stdin);
    if (ret == NULL) {
      close(sockfd); 
      exit(0);
    }
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
      printInternalError();
    }
  }
  close(sockfd); 
}
