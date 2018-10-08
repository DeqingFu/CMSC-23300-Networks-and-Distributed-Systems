#include "utils.h"
/* flag = 0 -> TCP
 * flag = 1 -> UDP */
void client_function(int port, char* hostname, char flag) {
  int sockfd, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[256];
  //char udp_buffer[1024];
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
  /* Now connect to the server */
  if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    printInternalError();
  }

  /*
  if (flag) {

    int len = 0;
    while (1) {
      bzero(buffer,256);
      char* ret = fgets(buffer,255,stdin);
      if (ret == NULL) {
        n = write(sockfd, udp_buffer, strlen(udp_buffer));
        if (n < 0) {
          printf("here error\n");
          printInternalError();
        }
        len = 0; 
        bzero(udp_buffer, 1024);
      } else {
        strcpy(udp_buffer+len, buffer);
        len += strlen(buffer);
      }  
      
    }
  } else {

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
  }
  */
  if (flag) {
    int sending = 1;
    while (1) {
      bzero(buffer,256);
      char* ret = fgets(buffer,255,stdin);
      if (ret == NULL) {
        close(sockfd); 
        //exit(0);
        sending = 0;
      }
      if (sending) {
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) {
          printInternalError();
        }
      }
    }
    close(sockfd); 
  } else {
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
