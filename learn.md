# 知识点：
    sysconf(_NPROCESSORS_ONLN) The number of processors currently online see also getnprocs_conf(3)
## signal
    当程序收到信号调用signal handler,系统是中断正在运行的进程，因此可以理解未signal的中断不是并行的。
