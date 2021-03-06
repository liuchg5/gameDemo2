#ifndef CSOCKETSRVEPOLL_H
#define CSOCKETSRVEPOLL_H

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>





#include "CSocketList.h"

#include "../common/common.h"
#include "../queue/CShmQueueSingle.h"

class CSocketSrvEpoll
{
public:
	int PORT_NUMBER; 
	int EPOLL_SIZE;
	int EPOLL_TIMEOUT;
	int LISTENQ;
	struct epoll_event ev;
	struct epoll_event * events;
	int epfd;
	int listenfd;
	struct sockaddr_in clientaddr;
	socklen_t clientAddrLen;
    struct sockaddr_in serveraddr;
    CSocketList socketlist;
	

public:
	CSocketSrvEpoll(int epoll_size, int epoll_timeout, int listenq);
	~CSocketSrvEpoll();
	
	int open(const char * serv_addr, int port_number);
	int myclose();

	int my_epoll_wait(CShmQueueSingle * pshmQueueSingle);

};




#endif
