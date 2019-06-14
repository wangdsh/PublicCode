# PublicCode

open source code<br>

sell_tickets_multi_thread.cpp 多线程卖票，线程间分工合作使用互斥锁<br>
sell_tickets_multi_process.cpp 多进程卖票，进程间通信使用共享内存技术，分工合作使用信号量技术。<br>
信号量的操作函数有两套，一套是Posix标准，另一套是System V标准。这里使用的是System V标准。<br>
