
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

    char tmp[30];
    int port;

    if (argc == 3)
    {
        strcpy(tmp, argv[1]);
        port = atoi(argv[2]);
    }
    else if (argc == 2)
    {
        strcpy(tmp, argv[1]);
        port = 12345;
    }
    else
    {
        strcpy(tmp, "127.0.0.1");
        port = 12345;
    }

    CShmQueueSingle sinq;
    sinq.crt(SQ5_SIZE, SQ5_FTOLK);
    sinq.get();
    sinq.init();
    sinq.clear();

    CShmQueueMulti mulq;
    mulq.crt(MQ6_SIZE, MQ6_FTOLK);
    mulq.get();
    mulq.init(GLOBAL_EPOLL_SIZE_DB);
    mulq.clear();

    CSocketSrvEpoll srv(GLOBAL_EPOLL_SIZE_DB, GLOBAL_EPOLL_TIMEOUT_DB, GLOBAL_EPOLL_LISTENQ_DB, &sinq, &mulq);//epollsize epolltimeout listenq
    srv.open(tmp, port);

    while (1)
    {
        if (srv.my_epoll_wait() < 0)
        {
            fprintf(stderr, "Err: db_insrv: srv.my_epoll_wait(&sinq, &mulq) < 0 \n");
            exit(-1);
        }
        if (mulq.popmsg_complex(&srv) < 0)
        {
            fprintf(stderr, "Err: db_insrv: mulq.popmsg_complex() < 0 \n");
            exit(-1);
        }
        usleep(IN_SLEEP_TIME_DB);

        // printf("usleep(IN_SLEEP_TIME_DB); \n");
    }
}