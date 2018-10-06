/* Deqing Fu, deqing
 * CS233, Autumn 2018
 * Project 1
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "utils.h"
int main(int argc, char* argv[]) {
  int opt;
  int option = 0; // Options to deal with -l -n options
  /* option = 0     -> None
   * option = 1     ->  -l
   * option = 2     ->  -l - n
   * option = 3     ->  -n
   * option = 4     -> invalid options
   */
  char hostname[256];
  memset(hostname, 0, sizeof(hostname) - 1);
  int port;
  
  /* Not printing error message of getopt when detecting invalid characters */
  opterr = 0; 

  /* Parsing options */
  while ((opt = getopt(argc, argv, "ln")) != -1) {
    switch(opt) 
    {
      case 'l':
        option = 1;
        break;
      case 'n':
        option = (option == 1) ? 2 : 3;
        break;
      case '?':
        option = 4;
        break;
      default:
        break;
    }
  }


  


  /* Dealing with different cases */
  switch(option) 
  {
    case 4:
      //invalid_format();
      break;
    
    case 0:
      if (argc < 3) {
        //invalid_format();
      } else {
        if (!(read_port(argc, argv, &port))) {
          // Do something
          fprintf(stderr, "Port: %d\n", port);
          strcpy(hostname, argv[1]);
          int sockfd, portno, n;
           struct sockaddr_in serv_addr;
           struct hostent *server;
           
           char buffer[256];
           
           /* Create a socket point */
           sockfd = socket(AF_INET, SOCK_STREAM, 0);
           
           if (sockfd < 0) {
              error("internal error\n");
           }
          
           server = gethostbyname(hostname);
           
           if (server == NULL) {
              error("internal error\n");
           }
           
           bzero((char *) &serv_addr, sizeof(serv_addr));
           serv_addr.sin_family = AF_INET;
           bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
           serv_addr.sin_port = htons(port);
           
           /* Now connect to the server */
           if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
              error("internal error\n");
           }
           
           /* Now ask for a message from the user, this message
              * will be read by server
           */
          
           printf("Please enter the message: ");
           bzero(buffer,256);
           fgets(buffer,255,stdin);
           
           /* Send message to the server */
           n = write(sockfd, buffer, strlen(buffer));
           if (n < 0) {
              error("internal error\n");
           }
  
          /*
          
          */
        } else {
          break;
        }
      }
      break;

    case 1:
      if (argc < 3) {
        //invalid_format();
      } else if (argc == 3) {
        if(!read_port(argc, argv, &port)) {
          // Do something
          //fprintf(stderr, "Port: %d\n", port);
          gethostname(hostname, 255);
          fprintf(stderr, "Hostname: %s\n", hostname);
          int sockfd, newsockfd;
          unsigned int clilen;
          char buffer[256];
          struct sockaddr_in serv_addr, cli_addr;
          int n;

          sockfd = socket(AF_INET, SOCK_STREAM, 0);

          if (sockfd < 0) {
            fprintf(stderr, "internal error\n");
            exit(1);
          } 

          bzero((char*) &serv_addr, sizeof(serv_addr));

          serv_addr.sin_family = AF_INET;
          serv_addr.sin_addr.s_addr = INADDR_ANY;
          serv_addr.sin_port = htons(port);
          if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            error("internal error");
          }
          listen(sockfd, 5);
          clilen = sizeof(cli_addr);
          newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
          if (newsockfd < 0) {
            error("internal error");
          }
          bzero(buffer, 256);
          n = read(newsockfd, buffer, 255);
          if (n < 0) {
            error("internal error");
          }
          fprintf(stdout, "%s", buffer);
        }
      } else if (argc == 4) {
        if(!read_port(argc, argv, &port)) {
          // Do something
          //fprintf(stderr, "Port: %d\n", port);
          //serv_addr.sin_port = htons(port);
          strcpy(hostname, argv[2]);
          fprintf(stderr, "%s\n", hostname);
        }
      } else {
        //invalid_format();
      }

    case 2:
      if (argc < 4) {
        //invalid_format();
      } else if (argc == 4) {
        if(!read_port(argc, argv, &port)) {
          // Do something
          //fprintf(stderr, "Port: %d\n", port);
          //serv_addr.sin_port = htons(port);
        }
      } else if (argc == 5) {
        if(!read_port(argc, argv, &port)) {
          // Do something
          //fprintf(stderr, "Port: %d\n", port);
          //serv_addr.sin_port = htons(port);
          strcpy(hostname, argv[3]);
          fprintf(stderr, "%s\n", hostname);
        }
      } else {
        //invalid_format();
      }

    case 3:
      if (argc < 3) {
        //invalid_format();
      } else if (argc == 3) {
        if(!read_port(argc, argv, &port)) {
          // Do something
          //fprintf(stderr, "Port: %d\n", port);
          //serv_addr.sin_port = htons(port);
        }
      } else if (argc == 4) {
        if(!read_port(argc, argv, &port)) {
          // Do something
          //fprintf(stderr, "Port: %d\n", port);
          //serv_addr.sin_port = htons(port);
          strcpy(hostname, argv[2]);
          fprintf(stderr, "%s\n", hostname);
        }
      } else {
        //invalid_format();
      }

    default:
      break;
  }
  
  return 0;
}