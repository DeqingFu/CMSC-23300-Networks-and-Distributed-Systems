#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdlib>
#include <queue>
#include <cctype>
#include <netinet/in.h>
#include <set>
#include <mutex>
#include <arpa/inet.h>
#include <netdb.h>

using namespace std;

// global variables shared by threads
int max_flows = -1;
int port = -1;
string hostname = "";
string local_directory = "";
struct hostent *server;
struct sockaddr_in serv_addr;
queue<string> q; //downloading queue
set<string> visited; //visited array


int main(int argc, char **argv) {
    int opt;
    while ((opt = getopt(argc, argv, "n:h:p:f:")) != -1) {
        switch (opt) {
            case 'n': 
                if (optarg) {
                    max_flows = atoi(optarg);
                }
                break;
            case 'h':
                if (optarg){
                    hostname = optarg;
                }
                break;
            case 'p':
                if (optarg){
                    port = atoi(optarg);
                }
                break;
            case 'f': 
                if (optarg) {
                    local_directory = optarg;
                }
                break;
            default:
                break;
        }
    }

    if (max_flows < 0 || port < 0 || hostname == "" || local_directory == "") {
        fprintf(stderr, "Invalid Arguments\n");
        cout << "./mcrawl [-n max-flows] [-h hostname] [-p port] [-f local_directory]" << endl;
        exit(1);
    }

    // server information completing
    server = gethostbyname(hostname.c_str());
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = port;

    // creating file
    errno = 0;
    int mkdir_error = mkdir(local_directory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (errno != EEXIST && mkdir_error == -1) {
        fprintf(stderr, "Error creating local directory\n");
        exit(1);
    }

    // Initializing downloading queue
    q.push("index.html");


    return 0;
}

