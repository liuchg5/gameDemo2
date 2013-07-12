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
#include "../common/myerror.h"
#include "../common/CStatistics.h"
#include "../queue/CShmQueueSingle.h"
#include "../queue/CShmQueueMulti.h"

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
	
	CStatistics sta;
	
	

public:
	CSocketSrvEpoll(int epoll_size, int epoll_timeout, int listenq);
	~CSocketSrvEpoll();
	
	int open(const char * serv_addr, int port_number);
	int myclose();
	int myclose(int index);

	int my_epoll_wait(CShmQueueSingle * pqs, CShmQueueMulti * pqm);
	
	// int my_epoll_wait_debug_nosend(CShmQueueSingle * pqs, CShmQueueMulti * pqm);
	
	// int my_epoll_wait_debug(CShmQueueSingle *pshmQueueSingle);

};




#endif
