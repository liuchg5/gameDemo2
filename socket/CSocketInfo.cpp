
#include "CSocketInfo.h"



CSocketInfo::CSocketInfo():
    srvfd(-1),
    clifd(-1),
    recvflag(0),
    sendflag(0),
    writable(0)
{
    memset(&recvmsg, 0, sizeof(recvmsg));
    memset(&sendmsg, 0, sizeof(sendmsg));
}

CSocketInfo::~CSocketInfo()
{

}

