# PublicCode

open source code<br>

name | description
---- | ---
sell_tickets_multi_thread.cpp | 多线程卖票，线程间分工合作使用互斥锁
sell_tickets_multi_process.cpp |  多进程卖票，进程间通信使用共享内存技术，分工合作使用信号量技术。信号量的操作函数有两套，一套是Posix标准，另一套是System V标准。这里使用的是System V标准。
sell_tickets_multi_machine |  跨机器卖票，通信使用连接的socket(TCP)，server端使用多线程处理请求。使用时client必须修改变量ip的值
webserver.cpp |  十分简陋的example，仅用于理解webserver工作原理
webserver-post-agent.cpp |  代理http post请求，可集成到爬虫系统
shell_scripts/distribute_order.sh |  在多个机器上执行同一命令，而不需要分别登录每台机器执行命令
send_email.py |  发送邮件，可用于代码执行完时提醒自己
