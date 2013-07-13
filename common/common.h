#ifndef COMMON_H
#define COMMON_H


#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define GLOBAL_EPOLL_SIZE 		10000
#define GLOBAL_EPOLL_TIMEOUT 	0
#define GLOBAL_EPOLL_LISTENQ 	500

#define IN_SLEEP_TIME	50
#define MID_SLEEP_TIME	50
#define OUT_SLEEP_TIME	50

#define GLOBAL_EPOLL_SIZE_DB		2
#define GLOBAL_EPOLL_TIMEOUT_DB 	0
#define GLOBAL_EPOLL_LISTENQ_DB 	2
#define IN_SLEEP_TIME_DB	50
#define MID_SLEEP_TIME_DB	50

// 吞吐量与队列长度息息相关！！！
#define SQ1_SIZE	(1024*1024*4)
#define SQ1_FTOLK	(1111)

#define MQ1_SIZE	(1024*1024*32)
#define MQ1_FTOLK	(2222)

#define SQ3_SIZE	(1024*1024*4)
#define SQ3_FTOLK	(3333)

#define SQ4_SIZE	(1024*1024*4)
#define SQ4_FTOLK	(4444)

#define SQ5_SIZE	(1024*1024*4)
#define SQ5_FTOLK	(5555)

#define MQ6_SIZE	(1024*1024*16)
#define MQ6_FTOLK	(6666)

// 设为非阻塞socket函数
int setnonblocking(int sock) ;


// 验证是否是BE的函数
int isBigEndian() ;

//set操作要在connect()或者listen()之前
int get_snd_size(int socketfd);
int get_rcv_size(int socketfd);
int set_snd_size(int socketfd, int size);
int set_rcv_size(int socketfd, int size);

// 信号处理函数，考虑可重入
void sig_SIGUSR1_op(int signum, siginfo_t *info, void *myact);
// 封装了安装信号函数
int my_sigaction(int sig,  void (*sig_op)(int, siginfo_t*, void*));

// 打开阻塞模型的socket
int open_socket(const char *serv_addr, int port_number);

//
unsigned int GetRandomInteger(int low, int up) ;

#endif

