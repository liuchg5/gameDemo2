


#include "../queue/CShmQueueSingle.h"
#include "../queue/CShmQueueMulti.h"
#include "../common/common.h"
#include "CGMEngine.h"

// #include "../common/CStatistics.h"

// debug
int test_CShmQueueSingle(CShmQueueSingle * pqto, CShmQueueSingle * pqfrom);


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
			gme.handle_db(&msgbuf, &mulq, &sinq_to_out);
		}

        // while (sinq.popmsg(&msgbuf) > 0)   // debug!!!
        // {
            // gme.handle_debug(&msgbuf, &mulq);
        // }
		
		// test single queue
		// if (test_CShmQueueSingle(&sinq_to_out, &sinq_from_out) < 0)  // debug!!!
		// {
			// printf("Err: test end \n");
			// return -1;
		// }

        usleep(MID_SLEEP_TIME);
    }
}

int test_CShmQueueSingle(CShmQueueSingle * pqto, CShmQueueSingle * pqfrom)
{
	StMsgBuffer tmpmsgbuf, tmpmsgbuf2;
    tmpmsgbuf.n = 0;

    CMsgHead *phead;

    CRequestUserInfoPara *poutpara =
        (CRequestUserInfoPara *)(tmpmsgbuf.buf + sizeof(CMsgHead));

    strcpy(poutpara->m_szUserName, "TestName");

    phead = (CMsgHead *)tmpmsgbuf.buf;
    phead->msglen = sizeof(CMsgHead) + sizeof(CMsgResponseLoginPara);
    phead->msgid = MSGID_REQUESTUSERINFO; //16位无符号整型，消息ID
    phead->msgtype = Request;   //16位无符号整型，消息类型，当前主要有Request、Response以及Notify三种类型
    phead->msgseq = 1234567890;     //32位无符号整型，消息序列号
    phead->srcfe = FE_GAMESVRD ;       //8位无符号整型，消息发送者类型，当前主要有FE_CLIENT、FE_GAMESVRD以及FE_DBSVRD三种
    phead->dstfe = FE_DBSVRD;     //8位无符号整型，消息接收者类型 同上
    phead->srcid = 0;   //16位无符号整型，当客户端向游戏服务器发送消息时ScrID为SessionID
    phead->dstid = 0;   //16位无符号整型，当游戏服务器向客户端发送消息是DstID为SessionID

	for (int i=0; i<100; ++i)
	{
		if (pqto->pushmsg(&tmpmsgbuf) <= 0)
		{
			printf("pqto->pushmsg() <= 0 \n");
			return -1;
		}
	}
	for (int i=0; i<100; ++i)
	{
		while (pqfrom->popmsg(&tmpmsgbuf2) <= 0)
		{
			// printf("pqfrom->popmsg() <= 0 \n");
			// return -1;
		}
	}
	return 0;
}
