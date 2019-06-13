#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>

using namespace std;

#define PROCESS_NUM 5
#define TIME_MEM_KEY 99
#define TIME_MEM_KEY2 199
#define TIME_SEM_KEY 9900
#define SEG_SIZE ((size_t)100) // size of segment
#define SEM_SIZE 1

int seg_id;
int seg_id2;
int semset_id;
pid_t parent_pid;

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
};

static int semaphore_p() {
    // 对信号量做减1操作，即等待P(sv)
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;
    sem_b.sem_flg = SEM_UNDO;
    if (semop(semset_id, &sem_b, 1) == -1) {
        //
        return 0;
    }
    return 1;
}

static int semaphore_v() {
    // 释放操作，使信号量变为可用，即发送信号V(sv)
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = SEM_UNDO;
    if (semop(semset_id, &sem_b, 1) == -1) {
        //
        return 0;
    }
    return 1;
}

// g++ sell_tickets_multi_process.cpp
int main() {

    int * mem_ptr;
    int * sold_array;
    parent_pid = getpid();

    // create a shared memory segment
    // 存放总票数
    seg_id = shmget(TIME_MEM_KEY, SEG_SIZE, IPC_CREAT|0777);

    // attach to it and get a pointer to where it attaches
    mem_ptr = (int *)shmat(seg_id, NULL, 0);
    mem_ptr[0] = 100; // 总票数

    // 存放每个售票点卖出的总票数
    seg_id2 = shmget(TIME_MEM_KEY2, SEG_SIZE, IPC_CREAT|0777);
    sold_array = (int *)shmat(seg_id2, NULL, 0);

    // create a semset, mode rw-rw-rw-
    // 信号量集合
    semset_id = semget(TIME_SEM_KEY, SEM_SIZE, (0666|IPC_CREAT|IPC_EXCL));

    union semun init_val;
    init_val.val = 1;
    semctl(semset_id, 0, SETVAL, init_val); // 第一个信号量的初始值设为1

    int fork_rv;
    for (int i=0; i<PROCESS_NUM; i++) { // 创建多个线程，每个线程代表一个售票点
        fork_rv = fork();
        if (fork_rv == -1) {
            cout << "fork error" << endl;
        } else if (fork_rv == 0) { // child process
            cout << "I'm " << i << " child. My pid is " << getpid() << " shared mem: " << mem_ptr[0] << endl;;
            int all_sold_num = 0;

            while(true) {

                // 进入临界区
                if (!semaphore_p()) {
                    break;
                }

                if (mem_ptr[0] > 0) { // 剩余票数
                    mem_ptr[0]--;
                    all_sold_num++;
                    cout << i << " sale finish, " << mem_ptr[0] << endl;

                    // 离开临界区
                    if (!semaphore_v()) {
                        // exit(EXIT_FAILURE);
                    }

                    //sleep(1);
                } else {
                    if (!semaphore_v()) {
                        // exit(EXIT_FAILURE);
                    }
                    // 漏掉了？
                    //cout << "Process: " << i << " sold " << all_sold_num << endl;
                    sold_array[i] = all_sold_num;
                    break;
                }
            }
            break;
        } else {
            cout << "I'm parent process. My pid is " << getpid() << endl;
        }
    }

    // 等待所有子进程执行完毕
    for (int i=0; i<PROCESS_NUM; i++) {
        wait(NULL);
    }

    if (parent_pid == getpid()) {
        cout << "tickets_num: " << mem_ptr[0]<< endl;
        for (int i=0; i<PROCESS_NUM; i++) {
            cout << "Process " << i << " sold " << sold_array[i] << " tickets." << endl;
        }
    }

    // clean up
    shmctl(seg_id, IPC_RMID, NULL);
    shmctl(seg_id2, IPC_RMID, NULL);
    semctl(semset_id, 0, IPC_RMID, NULL);

    return 0;
}
