#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

using namespace std;

#define CLIENT_NUM 5

int tickets_num = 100;
pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

void* sell_tickets(void* ptr) {

    int all_sold_num = 0;
    while(true) {
        pthread_mutex_lock(&counter_lock);
        if (tickets_num > 0) {
            tickets_num--;
            all_sold_num++;
            cout << "I'm " << pthread_self()<< ", sale finish, " << tickets_num << endl;
            pthread_mutex_unlock(&counter_lock);
            //sleep(1);
        } else {
            cout << "Thread: " << pthread_self() << " sold " << all_sold_num << endl;
            pthread_mutex_unlock(&counter_lock);
            break;
        }
    }
}

// g++ sell_tickets.cpp -lpthread
// 假如火车站有100张火车票要卖，现在有5个售票点同时售票，用5个线程模拟这5个售票点的售票情况。
int main() {
    pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t) * CLIENT_NUM);
    for (int i=0; i<CLIENT_NUM; i++) {
        pthread_create(&threads[i], NULL, sell_tickets, (void *)&i);
    }
    for (int i=0; i<CLIENT_NUM; i++) {
        pthread_join(threads[i], NULL);
    }
    cout << "tickets_num: " << tickets_num << endl;
    return 0;
}
