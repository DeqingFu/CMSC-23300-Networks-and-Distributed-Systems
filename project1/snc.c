/* Deqing Fu, deqing
 * CS233, Autumn 2018
 * Project 1
 */
#include "utils.h"

int main(int argc, char* argv[]) {
  int opt;
  int option = 0; // Options to deal with -l -n options
  /* option = 0     -> None
   * option = 1     ->  -l
   * option = 2     ->  -l - u
   * option = 3     ->  -u
   * option = 4     -> invalid options
   */
  char hostname[256];
  memset(hostname, 0, sizeof(hostname) - 1);
  int port;
  
  /* Not printing error message of getopt when detecting invalid characters */
  opterr = 0; 

  /* Parsing options */
  while ((opt = getopt(argc, argv, "lu")) != -1) {
    switch(opt) 
    {
      case 'l':
        option = (option == 3) ? 2 : 1;
        break;
      case 'u':
        option = (option == 1) ? 2 : 3;
        break;
      case '?':
        option = 4;
        break;
      default:
        break;
    }
  }
  //fprintf(stderr, "%d\n", option);
  /* Dealing with different cases */
  switch(option) 
  {
    case 4:
      invalid_format();
      break;
    
    case 0: // Client TCP
      if (argc < 3) {
        invalid_format();
      } else {
        if (!(read_port(argc, argv, &port))) {
          // Do something
          //fprintf(stderr, "Port: %d\n", port);
          strcpy(hostname, argv[1]);
          int sockfd, n;
          struct sockaddr_in serv_addr;
          struct hostent *server;
          char buffer[256];

          while (1) {
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
            bzero(buffer,256);
            char* ret = fgets(buffer,255,stdin);
            if (ret == NULL) {
              shutdown(sockfd,2);
              close(sockfd); 
              exit(0);
            }

            n = write(sockfd, buffer, strlen(buffer));
            if (n < 0) {
              printInternalError();
            }

            /* shutdown connection */
            shutdown(sockfd,2);
            close(sockfd); 
          }
        } else {
          break;
        }
      }
      break;

    case 1: // Server TCP
      if (argc < 3) {
        invalid_format();
      } else if (argc == 3) {
        if(read_port(argc, argv, &port)) { // read port failed
          invalid_format();
        }
      } else if (argc == 4) {
        if(!read_port(argc, argv, &port)) {
          strcpy(hostname, argv[2]);
          //fprintf(stderr, "hostname: %s\n", hostname);
        }
      } else {
        invalid_format();
      }

      /* main server functionalities, TCP */
      int sockfd, newsockfd, pid;
      unsigned int clilen;
      char buffer[256];
      struct sockaddr_in serv_addr, cli_addr;
      int n;

      sockfd = socket(AF_INET, SOCK_STREAM, 0);
      int option = 1;
      setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
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

      while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
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
        if (newsockfd < 0) {
          printInternalError();
        }

        if (server_read_and_print(newsockfd)) {
          close(newsockfd);
          break;
        }
        close(newsockfd);
      }    
      break;

    case 2:
      if (argc < 4) {
        invalid_format();
      } else if (argc == 4) {
        if(!read_port(argc, argv, &port)) {
          // Do something
        }
      } else if (argc == 5) {
        if(!read_port(argc, argv, &port)) {
          // Do something
          strcpy(hostname, argv[3]);
          fprintf(stderr, "%s\n", hostname);
        }
      } else {
        invalid_format();
      }
      break;

    case 3:
      if (argc < 3) {
        invalid_format();
      } else if (argc == 3) {
        if(!read_port(argc, argv, &port)) {
          // Do something
        }
      } else if (argc == 4) {
        if(!read_port(argc, argv, &port)) {
          // Do something
          strcpy(hostname, argv[2]);
          fprintf(stderr, "%s\n", hostname);
        }
      } else {
        invalid_format();
      }
      break;

    default:
      break;
  }
  return 0;
}