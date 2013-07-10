#ifndef CSOCKETINFO_H
#define CSOCKETINFO_H

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "../msg/StMsgBuffer.h"

class CSocketInfo //保存每个socket的数据
{
public:
    int srvfd;     // server_socket_fd
    int clifd; // client_socket_fd
    int recvflag, sendflag; // 接收和发送消息的标志, 0未接受到完整消息，1接受到完整消息
    StMsgBuffer recvmsg; // 缓存接收到的消息
    StMsgBuffer sendmsg; // 缓存要发送的消息
public:
    CSocketInfo();
    ~CSocketInfo();
    void clear()
    {
        srvfd = -1;
        clifd = -1;
        recvflag = 0;
        sendflag = 0;
        memset(&recvmsg, 0, sizeof(recvmsg));
        memset(&sendmsg, 0, sizeof(sendmsg));
    }
};




#endif
