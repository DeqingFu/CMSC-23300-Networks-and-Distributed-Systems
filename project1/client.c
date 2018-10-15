#include "utils.h"
/* flag = 0 -> TCP
 * flag = 1 -> UDP */
void client_function(int port, char* hostname, char flag) {
  int sockfd, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[256];
  /* Create a socket point */
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
  server = gethostbyname(hostname);
   
  if (server == NULL) {
    printInternalError();
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(port);

  if (!flag) { // TCP
    /* Now connect to the server if TCP*/
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      printInternalError();
    }
  }

  if (flag) { // UDP
    int sending = 1;
    while (1) {
      bzero(buffer,256);
      char* ret = fgets(buffer,255,stdin);
      if (ret == NULL) {
        close(sockfd); 
        sending = 0;
      }
      if (sending) {
        n = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (n < 0) {
          printInternalError();
        }
      }
    }
    close(sockfd); 
  } else { // TCP
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
  
}
