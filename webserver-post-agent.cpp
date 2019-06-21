// reference: http://www.voidcn.com/article/p-xcwmcfwi-bqm.html
// usage example by curl:
//     curl -X POST -H "Content-Type: text/plain" --data "rhttps://www.baidu.com" http://192.168.1.105:8089
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdexcept>
#include <algorithm>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

using namespace std;

const int BUFFSIZE = 12048;
const int MAXLINK = 10; // 未经过处理的连接请求队列可以容纳的最大数目
const int DEFAULT_PORT = 8089; // 端口号

// reference: https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-output-of-command-within-c-using-posix
string my_exec(const char* cmd) {
    char buffer[128];
    string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

// reference: https://www.cnblogs.com/wuchanming/p/4020463.html
void wait4children(int signo) {
    int status;
    wait(&status);
}

int main(int argc, char const *argv[]) {

    // 编写一个SIGCHLD信号处理程序来调用wait/waitpid来等待子进程返回
    signal(SIGCHLD, wait4children);

    int socket_fd, connect_fd;
    struct sockaddr_in servaddr;
    char buff[BUFFSIZE];

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        cout << "create socket error" << endl;
        return -1;
    }

    // 使程序退出后立即释放socket，解决Address already in use问题
    int option = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // IP必须是网络字节序，INADDR_ANY是绑定本机上所有IP
    servaddr.sin_port = htons(DEFAULT_PORT); // 端口号必须是网络字节序

    if (bind(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        cout << "bind error" << endl;
        return -1;
    }

    if (listen(socket_fd, MAXLINK) == -1) {
        cout << "listen error" << endl;
    }

    while(true) { // 循环等待请求
        cout << "loop... wait for request ..." << endl;

        connect_fd = accept(socket_fd, (struct sockaddr*)NULL, NULL);
        if (connect_fd == -1) {
            cout << "accept error" << endl;
        } else {
            cout << "connect_fd: " << connect_fd << endl;
        }

        memset(buff, '\0', sizeof(buff));

        //recv(connect_fd, buff, BUFFSIZE - 1, 0); // 把请求头（或发送的消息）写入buff中
        int size = read(connect_fd, buff, BUFFSIZE - 1); // 读取Http请求报文
        cout << "recive message from client: " << buff << endl;
        if (size > 0) {
            char method[5];
            char filename[50];
            int i=0, j=0;
            while(buff[j] != ' ' && buff[j] != '\0') { // 获取请求方法
                method[i++] = buff[j++];
            }
            j++;
            method[i] = '\0';

            // 获取请求文件
            i = 0;
            while(buff[j] != ' ' && buff[j] != '\0') {
                filename[i++] = buff[j++];
            }
            filename[i] = '\0';

            if (strcasecmp(method, "GET") == 0) {
                // response_get(filename);
            } else if (strcasecmp(method, "POST") == 0) {
                cout << "POST method" << endl;
                char argvs[2200];
                memset(argvs, 0, sizeof(argvs));
                int k = 0;
                char *ch = NULL;
                j++;

                int buff_len = string(buff).length();
                while((j < buff_len) && (ch = strcasestr(argvs, "Content-Length")) == NULL) {
                    k = 0;
                    memset(argvs, 0, sizeof(argvs));
                    while(buff[j] != '\r' && buff[j] != '\0') {
                        argvs[k++] = buff[j++];
                    }
                    j++;
                }

                // filter illegal request
                if (ch == NULL) {
                    cout << "illegal request!" << endl;
                    close(connect_fd);
                    continue;
                }

                int length;
                char *str = strchr(argvs, ':'); // 获取POST请求数据的长度
                str++;
                sscanf(str, "%d", &length);
                cout << "content-length: " << length << endl;

                j = strlen(buff) - length; // 从请求报文的尾部获取请求数据
                k = 0;
                memset(argvs, 0, sizeof(argvs));
                
                // simple validation
                if (buff[j] != 'r') {
                    cout << "illegal request!" << endl;
                    close(connect_fd);
                    continue;
                } else {
                    j++;
                }

                while(buff[j] != '\r' && buff[j] != '\0') {
                    argvs[k++] = buff[j++];
                }
                argvs[k] = '\0';
                cout << "argvs: " << argvs << endl;

                // let child process deal with request
                int ret_fork = 0;
                ret_fork = fork();
                if (ret_fork == -1) {
                    cout << "Fork error!" << endl;
                } else if (ret_fork == 0) { // child process
                    cout << "child process - connect_fd: " << connect_fd << endl;

                    // 发送响应给客户端
                    sprintf(buff, "HTTP/1.0 200 OK\r\n");
                    send(connect_fd, buff, strlen(buff), 0);

                    strcpy(buff, "Server: SimpleWebServer\r\n");
                    send(connect_fd, buff, strlen(buff), 0);

                    sprintf(buff, "Content-Type: text/html\r\n");
                    send(connect_fd, buff, strlen(buff), 0);

                    strcpy(buff, "\r\n");
                    send(connect_fd, buff, strlen(buff), 0);

                    memset(buff, '\0', sizeof(buff));

                    //string cmd = "curl https://www.baidu.com";
                    string cmd = "curl " + string(argvs);
                    string response = my_exec(cmd.c_str());
                    cout << "Output: " << response;
                    send(connect_fd, response.c_str(), response.length(), 0);

                    exit(0);
                } else { // parent process
                    //wait(NULL);
                    // do nothing
                }

            } else {
                cout << "unkown method" << endl;
            }
        }
        // 必须关闭连接，不然浏览器无法加载完毕
        close(connect_fd);

    }
}
