
#include "../socket/CSocketCli.h"

#include "../msg/StMsgBuffer.h"
#include "../common/common.h"
#include "../queue/CShmQueueSingle.h"
#include "../queue/CShmQueueMulti.h"
#include "../socket/CSocketSrvEpoll.h"

#include "../common/CStatistics.h"

int main(int argc, char ** argv)
{
	printf("Info: Begin outsrv !!! \n");
	
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
	
	CSocketCli cli;
	cli.open(tmp, port);
	
	CShmQueueSingle recvQ;
    recvQ.crt(SQ3_SIZE, SQ3_FTOLK);
    recvQ.get();
    recvQ.init();
    recvQ.clear();
	
	CShmQueueSingle sendQ;
    sendQ.crt(SQ4_SIZE, SQ4_FTOLK);
    sendQ.get();
    sendQ.init();
    sendQ.clear();
	
	while (1)
    {
        if (cli.recv_and_send(&recvQ, &sendQ) < 0)
		{
			fprintf(stderr, "Err: outsrv: cli.recv_and_send(&recvQ, &sendQ) < 0 \n");
			return -1;
		}
		
		// if (cli.recv_and_send_debug(&recvQ, &sendQ) < 0)  //debug
		// {
		// 	fprintf(stderr, "Err: cli.recv_and_send() failed! \n");
		// 	return -1;
		// }
		
        usleep(OUT_SLEEP_TIME);
		
		// printf("usleep(OUT_SLEEP_TIME); \n");
    }
}
