
#include "../msg/StMsgBuffer.h"
#include "../common/common.h"
#include "../queue/CShmQueueSingle.h"
#include "../queue/CShmQueueMulti.h"
#include "../socket/CSocketSrvEpoll.h"

#include "../common/CStatistics.h"


int main(int argc, char **argv)
{
    printf("Info: begin insrv! \n");
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
		port = 10203;
	}
	else
	{
		strcpy(tmp, "127.0.0.1");
		port = 10203;
	}

    CSocketSrvEpoll srv(GLOBAL_EPOLL_SIZE, GLOBAL_EPOLL_TIMEOUT, GLOBAL_EPOLL_LISTENQ);//epollsize epolltimeout listenq
    srv.open(tmp, port);

    CShmQueueSingle sinq;
    sinq.crt(1024 * 1024 * 4, 1111);
    sinq.get();
    sinq.init();
    sinq.clear();

    CShmQueueMulti mulq;
    mulq.crt(1024 * 1024 * 2, 2222);
    mulq.get();
    mulq.init(GLOBAL_EPOLL_SIZE);
    mulq.clear();

    while (1)
    {
        mulq.popmsg_complex(srv.epfd, &(srv.socketlist));
        srv.my_epoll_wait(&sinq);
        usleep(IN_SLEEP_TIME);
		
		// srv.my_epoll_wait_debug(&sinq);;//debug
		// usleep(IN_SLEEP_TIME);
    }
}