


#include "../queue/CShmQueueSingle.h"
#include "../queue/CShmQueueMulti.h"
#include "../common/common.h"
#include "CGMEngine.h"

#include "../common/CStatistics.h"




int main(int argc, char **argv)
{
	printf("Info: begin midsrv! \n");
	
    // 安装信号， 为了gprof
    // 使用kill -s SIGUSR1 pid
    my_sigaction(SIGUSR1, sig_SIGUSR1_op);


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

    CShmQueueSingle sinq_from_out;
    sinq_from_out.crt(1024 * 1024 * 1, 3333);
    sinq_from_out.get();
    sinq_from_out.init();
    sinq_from_out.clear();

    CShmQueueSingle sinq_to_out;
    sinq_to_out.crt(1024 * 1024 * 1, 4444);
    sinq_to_out.get();
    sinq_to_out.init();
    sinq_to_out.clear();

	
	
    CGMEngine gme(GLOBAL_EPOLL_SIZE);


    StMsgBuffer msgbuf;

    while (1)
    {
		while (sinq.popmsg(&msgbuf) > 0)
		{
			gme.handle_client(&msgbuf, &mulq, &sinq_to_out);
		}
		
		while (sinq_from_out.popmsg(&msgbuf) > 0)
		{
            // printf("sinq_from_out.popmsg(&msgbuf) \n");
			gme.handle_db(&msgbuf, &mulq, &sinq_to_out);
		}

        // while (sinq.popmsg(&msgbuf) > 0)   // debug!!!
        // {
        //     gme.handle_debug(&msgbuf, &mulq);
        // }



        usleep(MID_SLEEP_TIME);
    }
}
