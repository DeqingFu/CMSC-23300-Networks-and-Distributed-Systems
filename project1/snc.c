/* Deqing Fu, deqing
 * CS233, Autumn 2018
 * Project 1
 */
#include "utils.h"
#include "client.h"
#include "server.h"
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
          strcpy(hostname, argv[1]);
          client_function(port, hostname, 0);
        } else {
          invalid_format();
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
        } else {
          invalid_format();
        }
      } else {
        invalid_format();
      }
      /* main server functionalities, TCP */
      server_function(port, hostname, 0);
      break;

    case 2: // Server UDP
      if (argc < 4) {
        invalid_format();
      } else if (argc == 4) {
        if(read_port(argc, argv, &port)) { // read port failed
          invalid_format();
        }
      } else if (argc == 5) {
        if(!read_port(argc, argv, &port)) {
          // Do something
          strcpy(hostname, argv[3]);
          //fprintf(stderr, "%s\n", hostname);
        } else {
          invalid_format();
        }
      } else {
        invalid_format();
      }
      server_function(port, hostname, 1);
      break;

    case 3: // clinet UDP
      if (argc < 3) {
        invalid_format();
      } else if (argc == 3) {
        if(read_port(argc, argv, &port)) { // read port failed
          invalid_format();
        }
      } else if (argc == 4) {
        if(!read_port(argc, argv, &port)) {
          // Do something
          strcpy(hostname, argv[2]);
          //fprintf(stderr, "%s\n", hostname);
        } else {
          invalid_format();
        }
      } else {
        invalid_format();
      }
      client_function(port, hostname, 1);
      break;

    default:
      break;
  }
  return 0;
}