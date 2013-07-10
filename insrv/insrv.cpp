
#include "../msg/StMsgBuffer.h"
#include "../common/common.h"
#include "../queue/CShmQueueSingle.h"
#include "../queue/CShmQueueMulti.h"
#include "../socket/CSocketSrvEpoll.h"

#include "../common/CStatistics.h"


int main(int argc, char **argv)
{
    // 安装信号， 为了gprof
    // 使用kill -s SIGUSR1 pid
    my_sigaction(SIGUSR1, sig_SIGUSR1_op);

    CSocketSrvEpoll srv(GLOBAL_EPOLL_SIZE, 1000, 500);//epollsize epolltimeout listenq
    srv.open("192.168.234.128", 10203);

    CShmQueueSingle sinq;
    sinq.crt(1024 * 1024 * 16, 1111);
    sinq.get();
    sinq.init();
    sinq.clear();

    CShmQueueMulti mulq;
    mulq.crt(1024 * 1024 * 8, 2222);
    mulq.get();
    mulq.init(GLOBAL_EPOLL_SIZE);
    mulq.clear();

    while (1)
    {
        mulq.popmsg_complex(srv.epfd, &(srv.socketlist));
        srv.my_epoll_wait(&sinq);
        usleep(IN_SLEEP_TIME);
    }
}