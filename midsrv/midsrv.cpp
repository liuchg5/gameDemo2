


#include "../queue/CShmQueueSingle.h"
#include "../queue/CShmQueueMulti.h"
#include "../common/common.h"
#include "CMidHandle.h"

#include "../common/CStatistics.h"




int main(int argc, char **argv)
{

    // 安装信号， 为了gprof
    // 使用kill -s SIGUSR1 pid
    my_sigaction(SIGUSR1, sig_SIGUSR1_op);


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

    // CShmQueueSingle sinq_to_out;
    // sinq.crt(1024 * 1024 * 16, 3333);
    // sinq.get();
    // sinq.init();
    // sinq.clear();

    // CShmQueueSingle sinq_from_out;
    // sinq.crt(1024 * 1024 * 16, 4444);
    // sinq.get();
    // sinq.init();
    // sinq.clear();

    CMidHandle midhan(GLOBAL_EPOLL_SIZE);


    StMsgBuffer inmsgbuf;

    while (1)
    {
        while (sinq.popmsg(&inmsgbuf) > 0)
        {
            midhan.handle(&inmsgbuf, &mulq);
        }

        // while (sinq_from_out.popmsg(&inmsgbuf) > 0)
        // {
        //     midhan.handle(&inmsgbuf, &sinq_to_out);
        // }

        usleep(MID_SLEEP_TIME);
    }
}
