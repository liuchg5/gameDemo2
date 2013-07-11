


#include "../queue/CShmQueueSingle.h"
#include "../queue/CShmQueueMulti.h"
#include "../common/common.h"
#include "CDBEngine.h"

#include "../common/CStatistics.h"




int main(int argc, char **argv)
{

    // 安装信号， 为了gprof
    // 使用kill -s SIGUSR1 pid
    my_sigaction(SIGUSR1, sig_SIGUSR1_op);


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
	

    CDBEngine dbe(GLOBAL_EPOLL_SIZE_DB);


    StMsgBuffer msgbuf;

    while (1)
    {
		while (sinq.popmsg(&msgbuf) > 0)
		{
			dbe.handle(&msgbuf, &mulq);
		}

        usleep(MID_SLEEP_TIME_DB);
    }
}
