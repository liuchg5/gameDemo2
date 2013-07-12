
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
    sinq.crt(SQ1_SIZE, SQ1_FTOLK);  
    sinq.get();
    sinq.init();
    sinq.clear();

    CShmQueueMulti mulq;
    mulq.crt(MQ1_SIZE, MQ1_FTOLK);
    mulq.get();
    mulq.init(GLOBAL_EPOLL_SIZE);
    mulq.clear();

    while (1)
    {
        
        if (srv.my_epoll_wait(&sinq, &mulq) < 0)
		{
			fprintf(stderr, "Err: insrv: srv.my_epoll_wait(&sinq, &mulq) < 0 \n");
			exit(-1);
		}
		
		// srv.my_epoll_wait_debug_nosend(&sinq, &mulq);  //debug
		
        usleep(IN_SLEEP_TIME);
		// printf("usleep(IN_SLEEP_TIME); \n");
		
		// srv.my_epoll_wait_debug(&sinq);;//debug have problem
		// usleep(IN_SLEEP_TIME);
    }
}