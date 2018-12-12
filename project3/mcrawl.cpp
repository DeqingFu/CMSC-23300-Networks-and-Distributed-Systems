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
int html_total = 0;
int file_total = 0;

void crawl_html(string html) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)  {
        cout << "error" << endl;
        exit(1); 
    }
    char sending[128];
    char receiving[1024];
    snprintf(sending, sizeof(sending), "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n" , html.c_str() ,hostname.c_str());
    int n = sendto(sockfd, sending, strlen(sending), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    ofstream fs;
    fs.open("tmp.html");
    while (1) {
        memset(receiving, 0, sizeof(receiving));
        n = recv(sockfd, receiving, sizeof(receiving), 0);
        if (n == 0) {
            break;
        } else {
            //printf(receiving);
            fs << receiving;
            //fprintf(fd, "%s", receiving);
        }
    }
    fs.close();

    ifstream is("tmp.html");
    char buff[128];
    char c;
    while (is.get(c)) {
        memset(buff, 0, sizeof(buff));
        char sh[4];
        char sr[3];
        switch (c) {
            case 'H':
            case 'h':
                memset(sh, 0, sizeof(sh));
                is.read(sh, 3);
                if (!strcmp(sh, "REF") || !strcmp(sh, "ref")) {
                    int cnt = 0;
                    int flag = 0;
                    int idx = 0;
                    while(is.get(c)) {
                        if (c < 32 || c > 127 || c == 64) {
                            continue; //invalid character
                        }                  
                        if (c == '"') {
                            if (cnt == 1) {
                                string url = string(buff);
                                if (visited.count(url) == 0) {
                                    q.push(url);
                                }
                                break;
                            } else {
                                flag = 1;
                            } 
                            cnt ++;
                        } else {
                            if (flag) {
                                buff[idx++] = c;
                            }
                        }
                    }
                }
                break;
            case 'S':
            case 's':
                memset(sr, 0, sizeof(sr));
                is.read(sr, 2);
                if (!strcmp(sr, "RC") || !strcmp(sr, "rc")) {
                    int cnt = 0;
                    int flag = 0;
                    int idx = 0;
                    while(is.get(c)) {
                        if (c < 32 || c > 127 || c == 64) {
                            continue; //invalid character
                        }   
                        if (c == '"') {
                            if (cnt == 1) {
                                string url = string(buff);
                                if (visited.count(url) == 0) {
                                    q.push(url);
                                }
                                break;
                            } else {
                                flag = 1;
                            } 
                            cnt ++;
                        } else {
                            if (flag) {
                                buff[idx++] = c;
                            }
                        }
                    }
                }
                break;
        }
    }
    //fclose(fd);
    close(sockfd);
}

void download_file(string file_name) {
    return;
}

void crawl() {
    
    string url; 

    while (1) {
        if (q.empty()) {
            break;
        } else {
            url = q.front();
            q.pop();
            if (url[0] == '#' or !url.compare("/") or !url.compare("./")) {
                continue;
            } else {
                int left = url.find(hostname);
                if (url[0] == 'h') {
                    if (left == string::npos) {
                        continue;
                    }
                }
            }
            if (visited.count(url) != 0) {
                continue;
            }
            if (!url.substr(0, 3).compare("../")) {
                continue;
            }
            cout << url << endl;
            visited.insert(url);
            int dot_pos = url.rfind('.');
            if (dot_pos == -1) {
                continue;
            }
            string file_extension = url.substr(dot_pos+1, url.size()-dot_pos); 
            if (file_extension == "htm" || file_extension == "html") {
                html_total ++;
                crawl_html(url);
            } else {
                file_total ++;
                download_file(url);
            }
        }
    }
    cout << "html total: " << html_total << endl;
    cout << "file total: " << file_total << endl;
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

