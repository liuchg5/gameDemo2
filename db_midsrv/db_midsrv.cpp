


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
    sinq.crt(SQ5_SIZE, SQ5_FTOLK);
    sinq.get();
    sinq.init();
    sinq.clear();

    CShmQueueMulti mulq;
    mulq.crt(MQ6_SIZE, MQ6_FTOLK);
    mulq.get();
    mulq.init(GLOBAL_EPOLL_SIZE_DB);
    mulq.clear();
	

    CDBEngine dbe(GLOBAL_EPOLL_SIZE_DB);


    StMsgBuffer msgbuf;

    while (1)
    {
		while (sinq.popmsg(&msgbuf) > 0)
		{
			if (dbe.handle(&msgbuf, &mulq) < 0)
			{
				fprintf(stderr, "Err: db_midsrv: dbe.handle(&msgbuf, &mulq) < 0 \n");
				exit(-1);
			}
		}

        usleep(MID_SLEEP_TIME_DB);
		
		// printf("usleep(MID_SLEEP_TIME_DB); \n");
    }
}
