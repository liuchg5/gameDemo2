#ifndef CSOCKET_H
#define CSOCKET_H

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>

#include "../msg/StMsgBuffer.h"
#include "../queue/CShmQueueSingle.h"
#include "../common/common.h"

class CSocketCli
{
public:
	int clifd;

	int recvflag, sendflag; // ���պͷ�����Ϣ�ı�־, 0δ���ܵ�������Ϣ��1���ܵ�������Ϣ
    StMsgBuffer recvmsg; // ������յ�����Ϣ
    StMsgBuffer sendmsg; // ����Ҫ���͵���Ϣ

	
	
	CSocketCli();
	~CSocketCli();
	int open(const char * servAddr, int portNumber);
	int myclose();
	int recv_and_send(CShmQueueSingle * precvQ, CShmQueueSingle * psendQ);
	int is_connected();
};








#endif
