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
#include <algorithm>
#include <pthread.h>
#include <thread>
#include <cstdio>
#include <ctime>
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
int num_crawling = 0;
mutex mtx, mtx1, mtx2, mtx3, mtx4, mtx5;

int get_code(char* buff) {
    string msg = string(buff);
    int pos = msg.find("HTTP");
    pos += 9;
    string code = msg.substr(pos, 3);
    return stoi(code);
}


string change_name(string url) {
    if (url.size() < 2) {
        return url;
    }
    if (url[0] == '.' && url[1] == '/') {
        url.erase(0,2);
    }
    if (url[url.size()-1] == '/') {
        int pos = url.rfind('/');
        url.replace(pos, 5, ".html");
    }
    replace(url.begin(), url.end(), '/', '_');
    char ret[128];
    snprintf(ret, sizeof(ret), "%s/%s", local_directory.c_str() , url.c_str());
    return ret;
}


void crawl_html(string url, string cookie) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)  {
        cout << "error" << endl;
        if (max_flows == 1) {
            close(sockfd);
            exit(1); 
        } else {
            mtx.lock();
            q.push(url);
            mtx.unlock();
            close(sockfd);
            exit(0);
        }
    }
    char sending[1024];
    char receiving[2048];
    if (cookie == "") {
        snprintf(sending, sizeof(sending), "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n" , url.c_str() ,hostname.c_str());
    } else {
        snprintf(sending, sizeof(sending), "GET /%s HTTP/1.0\r\nHost: %s\r\nCookie: %s\r\n\r\n", url.c_str() ,hostname.c_str(), cookie.c_str());
    }
    int n = sendto(sockfd, sending, strlen(sending), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    string filename = change_name(url);
    ofstream fs;
    fs.open(filename);
    int writing_flag = 0;
    int code_flag = 0;
    
    string path = "";
    int idx_pos = url.find("/index.html");
    if (idx_pos != string::npos) {
        path = url.substr(0, idx_pos+1);
    }
    
    while (1) {
        memset(receiving, 0, sizeof(receiving));
        n = recv(sockfd, receiving, sizeof(receiving)-1, 0);
        receiving[n] = 0;
        string msg = string(receiving);
        if (!code_flag) {
            int code = get_code(receiving);
            if (code == 404) {
                close(sockfd);
                remove(filename.c_str());
                fs.close();
                return;
            }
            code_flag = 1;
        }
        if (n == 0) {
            break;
        } else {

            if (!writing_flag) {
                int pos = msg.find("\r\n\r\n");
                if (pos != -1) {
                    pos += 4;
                    fs << msg.substr(pos, msg.size() - pos).c_str();
                    writing_flag = 1;
                }
            } else {
                fs << receiving;
            }
        }
    }
    fs.close();
    
    ifstream is(filename);
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
                        if (c == '"' || c == 39) {
                            if (cnt == 1) {
                                string new_url = string(buff);
                                if (new_url[0] == '#' or !new_url.compare("/") or !new_url.compare("./")) break;
                                int left = url.find(hostname);
                                if (new_url[0] == 'h') {
                                    if (left == string::npos) {
                                        break;
                                    }
                                }
                                if (new_url.find("#") == string::npos && path != "") {
                                    string path_cpy = path;
                                    new_url = path_cpy.replace(path.size(), new_url.size(), new_url);
                                } 
                                if (visited.count(new_url) == 0) {
                                    mtx.lock();
                                    q.push(new_url);
                                    mtx.unlock();
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
                        if (c == '"' || c == 39) {
                            if (cnt == 1) {
                                string new_url = string(buff);
                                if (new_url[0] == '#' or !new_url.compare("/") or !new_url.compare("./")) break;
                                int left = url.find(hostname);
                                if (new_url[0] == 'h') {
                                    if (left == string::npos) {
                                        break;
                                    }
                                }
                                if (new_url.find("#") == string::npos && path != "") {
                                    string path_cpy = path;
                                    new_url = path_cpy.replace(path.size(), new_url.size(), new_url);
                                } 

                                if (visited.count(new_url) == 0) {
                                    mtx.lock();
                                    q.push(new_url);
                                    mtx.unlock();
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
    mtx3.lock();
    html_total ++;
    mtx3.unlock();
    is.close();
    close(sockfd);
}

void download_file(string url, string cookie) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)  {
        cout << "error" << endl;
        exit(1); 
    }
    char sending[1024];
    char receiving[4096];
    if (cookie == "") {
        snprintf(sending, sizeof(sending), "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n" , url.c_str() ,hostname.c_str());
    } else {
        snprintf(sending, sizeof(sending), "GET /%s HTTP/1.0\r\nHost: %s\r\nCookie: %s\r\n\r\n" , url.c_str() ,hostname.c_str(), cookie.c_str());
    }
    int n = sendto(sockfd, sending, strlen(sending), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    string filename = change_name(url);
    ofstream fs;
    fs.open(filename.c_str(), ios::out | ios::binary);
    int writing_flag = 0;
    int code_flag = 0;
    while (1) {
        memset(receiving, 0, sizeof(receiving));
        n = recv(sockfd, receiving, sizeof(receiving), 0);
        if (!code_flag) {
            int code = get_code(receiving);
            if (code == 404) {
                close(sockfd);
                remove(filename.c_str());
                fs.close();
                return;
            }
            code_flag = 1;
        }
        if (n == 0) {
            break;
        } else {
            if (!writing_flag) {
                int pos;
                
                for (pos = 0; pos < n-4; pos ++) {
                    if (receiving[pos] == '\r' && receiving[pos+1] == '\n' && receiving[pos+2] == '\r' && receiving[pos+3] == '\n') {
                        writing_flag = 1;
                        break;
                    } 
                }
                if (pos <= n) {
                    writing_flag = 1;
                }

                for (int i = pos + 4; i < n; i ++) {
                   fs << receiving[i];
               }
            } else {
                for (int i = 0; i < n; i ++) {
                    fs << receiving[i];
                }
            }
        }
    }
    mtx5.lock();
    file_total ++;
    mtx5.unlock();
    fs.close();
    close(sockfd);
    return;
}

string request_cookie() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)  {
        cout << "error" << endl;
        exit(1); 
    }
    char sending[1024];
    char receiving[2048];
    snprintf(sending, sizeof(sending), "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n" , "index.html" ,hostname.c_str());
    int n = sendto(sockfd, sending, strlen(sending), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    memset(receiving, 0, sizeof(receiving));
    n = recv(sockfd, receiving, sizeof(receiving)-1, 0);
    receiving[n] = 0;
    string msg = string(receiving);
    int left = msg.find("Set-Cookie");
    int right = msg.find("Vary");
    if (left == string::npos || right == string::npos) {
        return "";
    } else {
        left += 12;
        right -= 2;
        string cookie = msg.substr(left, right - left);
        return cookie;
    }
}

void crawl(int thread_id, string cookie) {
    string url; 
    while (1) {
        mtx.lock();
        if (q.empty()) {
            mtx1.lock();
            if (num_crawling > 0) {
                mtx1.unlock();
                mtx.unlock();
                continue;
            } else {
                mtx1.unlock();
                mtx.unlock();
                break;
            }
        } else {
            url = q.front();
            q.pop();
            mtx.unlock();
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
            
            int dot_pos = url.rfind('.');
            if (dot_pos == -1 && url.find('/') == -1) {
                continue;
            }

            mtx2.lock();
            visited.insert(url);
            mtx2.unlock();
            mtx1.lock();
            num_crawling ++;
            mtx1.unlock();
            string file_extension = url.substr(dot_pos+1, url.size()-dot_pos);
            if (file_extension == "htm" || file_extension == "html" || url[url.size()-1] == '/') {
                mtx4.lock();
                cout << "Tread " << thread_id << ": Crawling html " << url << endl;
                mtx4.unlock();
                crawl_html(url, cookie);
            } else {
                mtx4.lock();
                cout << "Tread " << thread_id << ": Crawling file " << url << endl;
                mtx4.unlock();
                download_file(url, cookie);
            }
            mtx1.lock();
            num_crawling --;
            mtx1.unlock();
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

    // Multi-threading 
    thread threads[max_flows];
    for (int t = 0; t < max_flows; t ++) {
        string cookie = request_cookie();
        threads[t] = thread(crawl, t, cookie); 
        usleep(100);
    }
    for (int t = 0; t < max_flows; t ++) {
        threads[t].join();
    }
    cout << html_total + file_total << " files crawled with ";
    cout << html_total << " html pages and "<< file_total << " contents"  << endl;
    return 0;
}