
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

    CSocketSrvEpoll srv(GLOBAL_EPOLL_SIZE_DB, GLOBAL_EPOLL_TIMEOUT_DB, GLOBAL_EPOLL_LISTENQ_DB);//epollsize epolltimeout listenq
    srv.open(tmp, port);

    CShmQueueSingle sinq;
    sinq.crt(1024 * 1024 * 1, 5555);
    sinq.get();
    sinq.init();
    sinq.clear();

    CShmQueueMulti mulq;
    mulq.crt(1024 * 1024 * 1, 6666);
    mulq.get();
    mulq.init(GLOBAL_EPOLL_SIZE_DB);
    mulq.clear();

    while (1)
    {
        mulq.popmsg_complex(srv.epfd, &(srv.socketlist));
        srv.my_epoll_wait(&sinq);
        usleep(IN_SLEEP_TIME_DB);
    }
}