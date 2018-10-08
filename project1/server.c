#include "utils.h"
/* flag = 0 -> TCP
 * flag = 1 -> UDP */
void server_function(int port, char* hostname, char flag) {  
  int sockfd, newsockfd;
  unsigned int clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n;

  if (flag) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  } else {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
  }
  
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
  if (flag) {
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
      int ret = server_read_and_print(sockfd, flag);
      if (ret == 1) {
        close(sockfd);
        break;
      } 
      if (ret == 2) {
        shutdown(sockfd, 2);
        break;
      }
      
    }
    
  } else {
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
      //printf("hello here\n");
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

      if (server_read_and_print(newsockfd, flag)) {
        close(newsockfd);
        break;
      }
      
    }
    close(newsockfd);
  }
}