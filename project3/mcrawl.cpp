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
#include <stdio.h>

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



void crawl_html(string html, int sockfd) {
    char sending[128];
    char receiving[1024];
    snprintf(sending, sizeof(sending), "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n" , html.c_str() ,hostname.c_str());
    int n = sendto(sockfd, sending, strlen(sending), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    //FILE *fd = fopen("tmp", "rw");
    while (1) {
        n = recv(sockfd, receiving, sizeof(receiving), 0);
        if (n == 0) {
            break;
        } else {
            //printf(receiving);
            cout << receiving;
            //fprintf(fd, "%s", receiving);
        }
        memset(receiving, 0, sizeof(receiving));
    }
    //fclose(fd);
    
    

    
    

}

void crawl() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)  {
        cout << "error" << endl;
        exit(1); 
    }
    string html; 
    while (1) {
        if (q.empty()) {
            break;
        } else {
            html = q.front();
            cout << html << endl;
            q.pop();
            crawl_html(html, sockfd);
        }
    }
}

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
    serv_addr.sin_port = htons(port);

    // creating file
    errno = 0;
    int mkdir_error = mkdir(local_directory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (errno != EEXIST && mkdir_error == -1) {
        fprintf(stderr, "Error creating local directory\n");
        exit(1);
    }

    // Initializing downloading queue
    q.push("index.html");
    crawl();
    return 0;
}

