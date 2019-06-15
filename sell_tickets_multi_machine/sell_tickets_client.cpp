#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <strings.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>

using namespace std;

#define LEN sizeof(int)
#define PORT 8089

int talk_with_server(int fd);
int connect_to_server(const char* host, int portnum);
int buy_one_ticket();

int main() {
    while(buy_one_ticket() != -1) {
        sleep(1);
    }
    return 0;
}

int buy_one_ticket() {

    int fd;
    string ip = "192.168.1.102"; // change ip for your network
    fd = connect_to_server(ip.c_str(), PORT);
    if (fd == -1) {
        cout << "connect to server fail." << endl;
        exit(1);
    } else {
        cout << "Connect to " << ip << ":" << PORT << " successful." << endl;
    }
    int rt_val = talk_with_server(fd);
    close(fd);
    return rt_val;
}

int connect_to_server(const char *host, int portnum) {

    // 创建一个socket
    int sock_id = socket(PF_INET, SOCK_STREAM, 0); // 跨机器，连接的
    if (sock_id == -1) {
        return -1;
    }

    struct sockaddr_in saddr; // build address here
    struct hostent * hp;

    // Connect to server
    bzero((void *)&saddr, sizeof(saddr));
    hp = gethostbyname(host);
    if (hp == NULL) {
        return -1;
    }

    bcopy((void *)hp->h_addr, (void *)&saddr.sin_addr, hp->h_length);
    saddr.sin_port = htons(portnum);
    saddr.sin_family = AF_INET;

    if (connect(sock_id, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) {
        return -1;
    }
    return sock_id;
}

int talk_with_server(int fd) {
    int val;
    int n;

    n = read(fd, &val, LEN);
    if (val == -1) {
        cout << "no tickets left." << endl;
    } else {
        cout << "ticket number: " << val << endl;
    }
    //write(1, &val, LEN);
    return val;
}
