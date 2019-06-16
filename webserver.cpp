/*
  十分简陋的example，仅用于理解webserver工作原理
*/
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netinet/in.h>
#include <fstream>

using namespace std;

const int BUFFSIZE = 1024;
const int MAXLINK = 10; // 未经过处理的连接请求队列可以容纳的最大数目
const int DEFAULT_PORT = 8089; // 端口号
 
/* 响应报文格式
<status-line>
<headers>
<blank line>
[<response-body>]

Response example:

HTTP/1.1 200 OK
Content-Type: text/html;charset=utf-8
Content-Length: 122
 
<html>
<head>
<title>Hello Server</title>
</head>
<body>
Hello Server
</body>
</html>
*/
void deal_get_http(int client_fd, char* buf) {

    /* 发送响应给客户端 */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    send(client_fd, buf, strlen(buf), 0);

    strcpy(buf, "Server: SimpleWebServer\r\n");
    send(client_fd, buf, strlen(buf), 0);

    sprintf(buf, "Content-Type: text/html\r\n");
    send(client_fd, buf, strlen(buf), 0);

    strcpy(buf, "\r\n");
    send(client_fd, buf, strlen(buf), 0);

    sprintf(buf, "<html><title>T</title><body>Hello World</body></html>");
    send(client_fd, buf, strlen(buf), 0);
}

/* 请求报文格式
<request-line>
<headers>
<blank line>
[<request-body]

Request example:

GET / HTTP/1.1
Host: 222.28.58.23:8089
Connection: keep-alive
Cache-Control: max-age=0
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*\/*;q=0.8,application/signed-exchange;v=b3
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9,en;q=0.8
Cookie: sidebar_collapsed=false; _xsrf=2|2dc8ac|69925cfebfe02f5e005ed1ee92095bd8|1553865506

*/
// 判断是否是GET请求
bool is_get_http(char* buff) {

    if (!strncmp(buff, "GET", 3)) {
        return true;
    } else {
        return false;
    }
}

int main(int argc, char const *argv[]) {

    int socket_fd, connect_fd;
    struct sockaddr_in servaddr;
    char buff[BUFFSIZE];
    int request_count = 0;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        cout<<"create socket error"<<endl;
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // IP必须是网络字节序，INADDR_ANY是绑定本机上所有IP
    servaddr.sin_port = htons(DEFAULT_PORT); // 端口号必须是网络字节序
    
    if (bind(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        cout<<"bind error"<<endl;
        return -1;
    }

    if (listen(socket_fd, MAXLINK) == -1) {
        cout<<"listen error"<<endl;
    }
    
    while(true) { // 循环等待请求
        connect_fd = accept(socket_fd, (struct sockaddr*)NULL, NULL);
        if (connect_fd == -1) {
            cout<<"accept error"<<endl;
        } else {
            request_count++;
            cout<<"Connect successful. request_count: " << request_count <<endl;
        }

        memset(buff, '\0', sizeof(buff));

        recv(connect_fd, buff, BUFFSIZE - 1, 0); // 把请求头（或发送的消息）写入buff中
        // 简单过滤一下
        if (!strncmp(buff, "GET /favicon.ico", 16)) {
            // do nothing
        } else {
            cout << "recive message from client: " << buff << endl;
        }

        if (is_get_http(buff)) { // deal with GET request
            deal_get_http(connect_fd, buff);
        }
        // 必须关闭连接，不然浏览器无法加载完毕
        close(connect_fd);  
    }

    close(socket_fd);

    return 0;
}
