
#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>



#include "common.h"


// 设为非阻塞socket函数
int setnonblocking(int sock) 
{
	int opts;
	opts = fcntl(sock, F_GETFL);
	if (opts < 0) {
		fprintf(stderr, "Err: fcntl(sock, F_GETFL)  \n");
		return -1;
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(sock, F_SETFL, opts) < 0) {
		fprintf(stderr, "Err: fcntl(sock, F_SETFL, opts)  \n");
		return -1;
	}
	return 0;
}

// 验证是否是BE的函数
int isBigEndian() 
{
	union {
		unsigned int a;
		unsigned char b;
	} c;
	c.a = 1;
	if (c.b == 1)
		return 1; // should be 1
	else
		return 0;
}

int get_snd_size(int socketfd)
{
	socklen_t optlen;    /* 选项值长度 */ 
	int snd_size = 0;   /* 发送缓冲区大小 */
	int err = -1;        /* 返回值 */ 
	
	optlen = sizeof(snd_size);
	err = getsockopt(socketfd, SOL_SOCKET, SO_SNDBUF, &snd_size, &optlen); 
    if(err<0){ 
        fprintf(stderr, "获取发送缓冲区大小错误\n"); 
		return -1;
    }
	return snd_size;
}
int get_rcv_size(int socketfd)
{
	socklen_t optlen;    /* 选项值长度 */ 
	int size = 0;   /* 发送缓冲区大小 */
	int err = -1;        /* 返回值 */ 
	
	optlen = sizeof(size);
	err = getsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, &size, &optlen); 
    if(err<0){ 
        fprintf(stderr, "获取接收缓冲区大小错误\n"); 
		return -1;
    }
	return size;
}
int set_snd_size(int socketfd, int size)
{
	socklen_t optlen;    /* 选项值长度 */ 
	int err = -1;        /* 返回值 */ 
	
	optlen = sizeof(size);
	err = setsockopt(socketfd, SOL_SOCKET, SO_SNDBUF, &size, optlen); 
    if(err<0){ 
        fprintf(stderr, "设置发送缓冲区大小错误\n"); 
		return -1;
    }
	return 0;
}
int set_rcv_size(int socketfd, int size)
{
	socklen_t optlen;    /* 选项值长度 */ 
	int err = -1;        /* 返回值 */ 
	
	optlen = sizeof(size);
	err = setsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, &size, optlen); 
    if(err<0){ 
        fprintf(stderr, "设置接收缓冲区大小错误\n"); 
		return -1;
    }
	return 0;
}


// 信号处理函数，考虑可重入
void sig_SIGUSR1_op(int signum, siginfo_t *info, void *myact)
{
	fprintf(stdout, "Info: kill by SIGUSR1 \n");
	exit(0);
}
// 封装了安装信号函数
int my_sigaction(int sig,  void (*sig_op)(int, siginfo_t*, void*))
{
	struct sigaction act;	
	
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = sig_op;
	
	if(sigaction(sig, &act, NULL) < 0)
	{
		fprintf(stderr, "Err: install sigal error\n");
		return -1;
	}
	return 0;
}


// 打开阻塞模型的socket
int open_socket(const char *serv_addr, int port_number)
{
	int socketfd;
	int PORT_NUMBER;
	char SERV_ADDR[30];
	struct sockaddr_in remote_addr;
	
	memset(&remote_addr, 0, sizeof(remote_addr));
   
	PORT_NUMBER = port_number;
	strcpy(SERV_ADDR, serv_addr);
	
	remote_addr.sin_family = AF_INET; //设置为IP通信  
	remote_addr.sin_addr.s_addr = inet_addr(SERV_ADDR); //服务器IP地址  
	remote_addr.sin_port = htons(PORT_NUMBER); //服务器端口号  
	
	/*创建客户端套接字--IPv4协议，面向连接通信，TCP协议*/
	if ((socketfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Err: open_socket socket(PF_INET, SOCK_STREAM, 0) \n ");
		fprintf(stderr, "Err: open_socket errno = %d (%s) \n ", errno, strerror(errno));
		return -1;
	}
	
	/*将套接字绑定到服务器的网络地址上*/
	if (connect(socketfd, (struct sockaddr *) &remote_addr, sizeof(struct sockaddr)) < 0) {
		fprintf(stderr, "Err: open_socket connect(socketfd, (struct sockaddr *) &remote_addr, sizeof(struct sockaddr) \n ");
		fprintf(stderr, "Err: open_socket errno = %d (%s) \n ", errno, strerror(errno));
		return -1;
	}
	return socketfd;
}

//
unsigned int GetRandomInteger(int low, int up) 
{ 
   unsigned int uiResult; 
 
   if (low > up) 
   { 
      int temp = low; 
      low = up; 
      up = temp; 
   } 
	srand((unsigned int)(time(NULL)));
   
   uiResult = low+(up-low)*rand()/(RAND_MAX + 1.0); 
 
   return uiResult; 
} 

