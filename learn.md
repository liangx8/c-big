# 查询当前有几块核
sysconf(_NPROCESSORS_ONLN) The number of processors currently online see also getnprocs_conf(3)
# signal
当程序收到信号调用signal handler,系统是中断正在运行的进程，因此可以理解未signal的中断不是并行的。
# 结构体字段地址对齐
强制要求结果以n字节内存对齐

    #progma pack(n)
# pthread_cond_wait()
能够被同线程的pthread_cond_signal()唤醒，例如signal handler是抢夺了当前的线程到执行。可以理解为与当前线程是相同到。在handler中使用pthread_cond_signal能唤醒同线程下到pthread_cond_wait()

# 路径操作的函数
    mkdir() man3
    opendir() man3
    stat()   man2