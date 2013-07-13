#include "CSocketCli.h"


CSocketCli::CSocketCli():
    clifd(-1),
    recvflag(0), // 表示recvmsg为空
    sendflag(0)  // 表示sendmsg为空
{
    memset(&recvmsg, 0, sizeof(recvmsg));
    memset(&sendmsg, 0, sizeof(sendmsg));
}

CSocketCli::~CSocketCli()
{

}

int CSocketCli::open(const char *servAddr, int portNumber)
{
    int port_number = portNumber;
    char srv_addr[30];
    strcpy(srv_addr, servAddr);

    struct sockaddr_in serveraddr;

    serveraddr.sin_family = AF_INET; //设置为IP通信
    serveraddr.sin_addr.s_addr = inet_addr(srv_addr); //服务器IP地址
    serveraddr.sin_port = htons(port_number); //服务器端口号

    /*创建客户端套接字--IPv4协议，面向连接通信，TCP协议*/
    if ((clifd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Err: CSocketCli socket(PF_INET, SOCK_STREAM, 0) failed! \n ");
        fprintf(stderr, "Err: CSocketCli errno = %d (%s) \n ", errno, strerror(errno));
        return -1;
    }

    // 设为非阻塞
    if (setnonblocking(clifd) != 0)
    {
        fprintf(stderr, "Err: CSocketCli setnonblocking() failed!\n ");
        fprintf(stderr, "Err: CSocketCli errno = %d (%s) \n ", errno, strerror(errno));
    }

    /*将套接字绑定到服务器的网络地址上*/
    if (connect(clifd, (struct sockaddr *) &serveraddr, sizeof(struct sockaddr)) < 0)
    {
        if (errno != EINPROGRESS)
        {
            fprintf(stderr, "Err: CSocketCli connect() failed! \n ");
            fprintf(stderr, "Err: CSocketCli errno = %d (%s) \n ", errno, strerror(errno));
            return -1;
        }
        else
        {
            fprintf(stderr, "Warn: CSocketCli connect() is in progress... \n");
        }
    }
    else
    {
        fprintf(stdout, "Info: CSocketCli connect() finished! \n");
    }
    return 0;
}
int CSocketCli::myclose()
{
    close(clifd);
    //TODO
    clifd = -1;
    memset(&recvmsg, 0, sizeof(recvmsg));
    memset(&sendmsg, 0, sizeof(sendmsg));
    sendflag = 0;
    recvflag = 0;
    return 0;
}

// #define RECV_WAIT_TIME 1000
int CSocketCli::recv_and_send(CShmQueueSingle *precvQ, CShmQueueSingle *psendQ)
{
    //一般要通过select来判断，但这里简化处理，假设成功
    //一次处理完

	static int recv_wait_time = 0; //debug
	
    ssize_t n;
    uint32_t msglen;

    // 处理接收
    if (recvflag == 1) //处理上次网络没发送完的
    {
        if (precvQ->pushmsg(&recvmsg) >= 0)
        {
            recvmsg.n = 0;
            recvflag = 0;
        }
        else
        {
            fprintf(stderr, "Err: CSocketCli recvQ is full < 0 !!!!!!\n ");
			return -1;
        }
    }
    while (recvflag == 0)
    {
        if (recvmsg.n < 4)
        {
            n = recv(clifd, recvmsg.buf + recvmsg.n, 4 - recvmsg.n, 0);
            if (n > 0)
            {
                recvmsg.n += n;
            }
            else if (n < 0)
            {
                if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    #ifdef RECV_WAIT_TIME
                    if (recv_wait_time++ > RECV_WAIT_TIME){
						fprintf(stderr, "Err:CSocketCli recv wait %d times !! \n", RECV_WAIT_TIME);
						recv_wait_time = 0;
					}
                    #endif
                    break;
                }
                else if (errno == ECONNRESET)
                {
                    fprintf(stderr, "Err: CSocketCli ECONNRESET \n ");
                    myclose();
                    return -1;
                }
                else
                {
                    fprintf(stderr, "Err: CSocketCli recv n < 0 but donot why \n ");
                    fprintf(stderr, "Err: CSocketCli errno = %d (%s) \n ", errno, strerror(errno));
                    myclose();
                    return -1;
                }
            }
            else if (n == 0)
            {
                fprintf(stderr, "Err: CSocketCli recv n == 0 \n ");
                myclose();
                return -1;
            }
        }
        if (recvmsg.n >= 4) // recvmsg.n >= 4
        {
            msglen = *(uint32_t *)recvmsg.buf;
            n = recv(clifd, recvmsg.buf + recvmsg.n, msglen - recvmsg.n, 0);
            if (n > 0)
            {
                recvmsg.n += n;
                if (msglen == recvmsg.n) // 读取完了msg
                {
                    if (precvQ->pushmsg(&recvmsg) >= 0)
                    {
                        recvmsg.n = 0;
                        recvflag = 0;
                    }
                    else
                    {
                        fprintf(stderr, "Err: CSocketCli recvQ is full < 0 !!!!!!\n ");
                        recvflag = 1;
						return -1;
                    }
                }
            }
            else if (n < 0)
            {
                if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    #ifdef RECV_WAIT_TIME
                    if (recv_wait_time++ > RECV_WAIT_TIME){
						fprintf(stderr, "Err:CSocketCli recv wait %d times !! \n", RECV_WAIT_TIME);
						recv_wait_time = 0;
					}
                    #endif
                    break;
                }
                else if (errno == ECONNRESET)
                {
                    fprintf(stderr, "Err: CSocketCli ECONNRESET \n ");
                    myclose();
                    return -1;
                }
                else
                {
                    fprintf(stderr, "Err: CSocketCli recv n < 0 but donot why \n ");
                    fprintf(stderr, "Err: CSocketCli errno = %d (%s) \n ", errno, strerror(errno));
                    myclose();
                    return -1;
                }
            }
            else if (n == 0)
            {
                fprintf(stderr, "Err: CSocketCli recv n == 0 \n ");
                myclose();
                return -1;
            }
        }
    } // while (recvflag == 0)

    // 处理发送
    if (sendflag == 0)
    {
        if (psendQ->popmsg(&sendmsg) >= 0)
        {
            sendmsg.n = 0;
            sendflag = 1;
        }
        else
        {
            ;
        }

    }
    while (sendflag == 1)
    {
        msglen = *(uint32_t *)sendmsg.buf;
        n = send(clifd, sendmsg.buf + sendmsg.n, msglen - sendmsg.n, 0);
        if (n < 0)
        {
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
            {
                printf("send() have to wait\n");
				// return -1;
                break;
            }
            else if (errno == ECONNRESET)
            {
                fprintf(stderr, "Err: CSocketCli ECONNRESET \n ");
                myclose();
                return -1;
            }
            else
            {
                fprintf(stderr, "Err: CSocketCli recv n < 0 but donot why \n ");
                fprintf(stderr, "Err: CSocketCli errno = %d (%s) \n ", errno, strerror(errno));
                myclose();
                return -1;
            }
        }
        else if (n == 0)
        {
            fprintf(stderr, "Err: CSocketCli send() n == 0 \n");
        }
        else if (n > 0)
        {
            sendmsg.n += n;
            if (msglen == sendmsg.n)  // 发送完了，取新msg
            {
                if (psendQ->popmsg(&sendmsg) >= 0)
                {
                    sendflag = 1;
                    sendmsg.n = 0;
                }
                else
                {
                    sendflag = 0;
					sendmsg.n = 0;
                }
            }
        }
    } // while (sendflag == 1)
    return 0;
}

int CSocketCli::is_connected()
{
    fd_set rset, wset;
    struct timeval waitd;
    int ret;

    waitd.tv_sec = 1;     // Make select wait up to 1 second for data
    waitd.tv_usec = 0;    // and 0 milliseconds.
    FD_ZERO(&rset); // Zero the flags ready for using
    FD_ZERO(&wset);
    FD_SET(clifd, &rset);
    FD_SET(clifd, &wset);

    ret = select(clifd + 1, &rset, &wset, NULL, &waitd);
    if (ret == 0) //timeout
    {
        return 0;
    }

    if (FD_ISSET(clifd, &rset) || FD_ISSET(clifd, &wset))
    {
        //Socket read or write available
        // int len = sizeof(error);
        // if (getsockopt(clifd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
        // {
        // return -1;
        // }
    }
    else
    {
        return 0;
    }
    return 1;
}




int CSocketCli::recv_and_send_debug(CShmQueueSingle *precvQ, CShmQueueSingle *psendQ)
{
    //一般要通过select来判断，但这里简化处理，假设成功
    //一次处理完


    while (psendQ->popmsg(&sendmsg) > 0)
    {

    CMsgHead *phead = (CMsgHead *)sendmsg.buf;
    uint16_t srcid = phead->srcid;
    uint16_t dstid = phead->dstid;
    // printf("recv_and_send_debug sendmsg srcid = %d\n", srcid);
    // printf("recv_and_send_debug sendmsg dstid = %d\n", dstid);

    CResponseUserInfoPara *poutpara =
        (CResponseUserInfoPara *)(recvmsg.buf + sizeof(CMsgHead));

    // pinpara->print(); // debug
	

    strcpy(poutpara->m_stPlayerInfo.m_szUserName, "TestName");
    poutpara->m_stPlayerInfo.m_unUin = 1;
    poutpara->m_stPlayerInfo.m_bySex = 0;
    poutpara->m_stPlayerInfo.m_unLevel = 99;
    poutpara->m_stPlayerInfo.m_unWin = 90;
    poutpara->m_stPlayerInfo.m_unLose = 8;
    poutpara->m_stPlayerInfo.m_unRun = 1;
    strcpy(poutpara->m_szPwd, "password");
    poutpara->m_bResultID = 3;


    phead = (CMsgHead *)recvmsg.buf;
    phead->msglen = sizeof(CMsgHead) + sizeof(CResponseUserInfoPara);
    // printf("phead->msglen = %d\n", phead->msglen);
    phead->msgid = MSGID_REQUESTUSERINFO; //16位无符号整型，消息ID
    phead->msgtype = Response;   //16位无符号整型，消息类型，当前主要有Requst、Response以及Notify三种类型
    phead->msgseq = 1234567890;     //32位无符号整型，消息序列号
    phead->srcfe = FE_DBSVRD ;       //8位无符号整型，消息发送者类型，当前主要有FE_CLIENT、FE_GAMESVRD以及FE_DBSVRD三种
    phead->dstfe = FE_GAMESVRD;     //8位无符号整型，消息接收者类型 同上
    phead->srcid = dstid;   //16位无符号整型，当客户端向游戏服务器发送消息时ScrID为SessionID
    phead->dstid = srcid;   //16位无符号整型，当游戏服务器向客户端发送消息是DstID为SessionID

    // printf("recv_and_send_debug recvmsg srcid = %d\n", phead->srcid);
    // printf("recv_and_send_debug recvmsg dstid = %d\n", phead->dstid);





        if (precvQ->pushmsg(&recvmsg) < 0)
        {
            fprintf(stderr, "Err: CSocketCli recvQ is full < 0 !!!!!!\n ");
        }

    }


    return 0;
}