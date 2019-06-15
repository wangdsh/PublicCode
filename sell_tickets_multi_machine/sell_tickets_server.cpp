#include <iostream>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>

using namespace std;

#define PORT 8089
#define HOSTLEN 256

int make_server_socket(int *sock_id, int portnum);
void process_request(int fd);

int tickets_num = 100;
pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

// g++ sell_tickets_server.cpp -lpthread
int main() {
    int sock, fd;
    make_server_socket(&sock, PORT);
    
    if (sock == -1) {
        cout << "make_server_socket error." << endl;
        exit(1);
    } else {
        cout << "make_server_socket succeed." << endl;
    }

    while(true) {
        fd = accept(sock, NULL, NULL); // take the next call
        if (fd == -1) {
            cout << "fd == -1" << endl;
            break;
        }

        process_request(fd);
        //shutdown(fd, SHUT_WR);
        close(fd);
        cout << "close fd: " << fd << endl;
    }
    return 0;
}

int make_server_socket(int *sock_id, int portnum) {

    // 创建一个socket
    *sock_id = socket(PF_INET, SOCK_STREAM, 0); // 跨机器，连接的
    if (*sock_id == -1) {
        return -1;
    }

    // 使程序退出后立即释放socket，解决Address already in use问题
    // https://stackoverflow.com/questions/5106674/error-address-already-in-use-while-binding-socket-with-address-but-the-port-num
    int option = 1;
    setsockopt(*sock_id, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    /* build address and bind it to socket*/
    struct sockaddr_in saddr; // build address here
    char hostname[HOSTLEN];
    struct hostent * hp;

    // clear the struct
    bzero((void *)&saddr, sizeof(saddr));
    gethostname(hostname, HOSTLEN);
    hp = gethostbyname(hostname);

    bcopy((void *)hp->h_addr, (void *)&saddr.sin_addr, hp->h_length);
    saddr.sin_port = htons(portnum);
    saddr.sin_family = AF_INET;

    // 给socket绑定一个地址
    if (bind(*sock_id, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) {
        return -1;
    }

    // 监听接入请求
    if (listen(*sock_id, 10) != 0) {
        return -1;
    }

    // print IP and port of socket
    char **pptr = hp->h_addr_list;
    char str[INET_ADDRSTRLEN];
    for (; *pptr != NULL; ++pptr) {
        cout << inet_ntop(hp->h_addrtype, *pptr, str, sizeof(str)) << ":" << portnum << endl;
    }

    return *sock_id;
}

void* sell_tickets(void* ptr) {
    int *fd = (int *) ptr;
    //cout << "after: fd is " << *fd << endl;
    pthread_mutex_lock(&counter_lock);
    if (tickets_num > 0) {
        int temp = tickets_num;
        tickets_num--;
        cout << "I'm thread " << pthread_self()<< ", tickets left: " << tickets_num << endl;
        pthread_mutex_unlock(&counter_lock);
        write(*fd, &temp, sizeof(int));
    } else {
        pthread_mutex_unlock(&counter_lock);
        int temp = -1;
        write(*fd, &temp, sizeof(int));
    }
}

void process_request(int fd) {
    //cout << "before: fd is " << fd << endl;
    pthread_t threadp;
    pthread_create(&threadp, NULL, sell_tickets, (void *)&fd);
    if (pthread_join(threadp, NULL)) {
        cout << "thread is not exit..." << endl;
    } else {
        cout << "thread exit." << endl;
    }
    //char buff[] = {'a', 'b', 'c', '\n'};
    //write(fd, buff, 4);
}
