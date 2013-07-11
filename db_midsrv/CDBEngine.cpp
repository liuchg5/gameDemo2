#include "CDBEngine.h"




CDBEngine::CDBEngine(int epoll_size):
    EPOLL_SIZE(epoll_size),
    map(epoll_size)
{

}
CDBEngine::~CDBEngine()
{

}

int CDBEngine::handle(StMsgBuffer * pmsg, CShmQueueMulti * pqm)
{
    CMsgHead *phead = (CMsgHead *)pmsg;
    uint16_t srcid = phead->srcid;
    uint16_t dstid = phead->dstid;
	switch (phead->msgid)
    {
    case MSGID_I2M_NEW_CONNECT:
        if (map.reg(srcid, dstid) < 0)
        {
            fprintf(stderr, "Err: map.reg() return neg val !!!\n");
        }
        break;

    case MSGID_I2M_CLO_CONNECT:
        if (map.del(srcid) < 0)
        {
            fprintf(stderr, "Err: map.del() return neg val !!\n");
        }
        break;

    case MSGID_REQUESTUSERINFO:
        return handle_RequestUserInfo(pmsg, pqm);
        break;
		
    default:
		fprintf(stderr, "Err: CDBEngine::handle() wrong msgid !!\n");
        return -1;
        break;
    }
    return 0;
}



	
int CDBEngine::handle_RequestUserInfo(StMsgBuffer * pmsg, CShmQueueMulti * pqm)
{
   StMsgBuffer tmpmsgbuf;
    tmpmsgbuf.n = 0;

    int index;

    CMsgHead *phead = (CMsgHead *)pmsg;
    uint16_t srcid = phead->srcid;
    uint16_t dstid = phead->dstid;

    CRequestUserInfoPara *pinpara =
        (CRequestUserInfoPara *)(pmsg->buf + sizeof(CMsgHead)); //char        m_szUserName[64];
    CResponseUserInfoPara *poutpara =
        (CResponseUserInfoPara *)(tmpmsgbuf.buf + sizeof(CMsgHead));

    // pinpara->print(); // debug
	

    strcpy(poutpara->m_stPlayerInfo.m_szUserName, pinpara->m_szUserName);
    poutpara->m_stPlayerInfo.m_unUin = 1;
    poutpara->m_stPlayerInfo.m_bySex = 0;
    poutpara->m_stPlayerInfo.m_unLevel = 99;
    poutpara->m_stPlayerInfo.m_unWin = 90;
    poutpara->m_stPlayerInfo.m_unLose = 8;
    poutpara->m_stPlayerInfo.m_unRun = 1;
    strcpy(poutpara->m_szPwd, "password");
    poutpara->m_bResultID = 3;


    phead = (CMsgHead *)tmpmsgbuf.buf;
    phead->msglen = sizeof(CMsgHead) + sizeof(CResponseUserInfoPara);
    // printf("phead->msglen = %d\n", phead->msglen);
    phead->msgid = MSGID_REQUESTUSERINFO; //16位无符号整型，消息ID
    phead->msgtype = Response;   //16位无符号整型，消息类型，当前主要有Requst、Response以及Notify三种类型
    phead->msgseq = 1234567890;     //32位无符号整型，消息序列号
    phead->srcfe = FE_DBSVRD ;       //8位无符号整型，消息发送者类型，当前主要有FE_CLIENT、FE_GAMESVRD以及FE_DBSVRD三种
    phead->dstfe = FE_GAMESVRD;     //8位无符号整型，消息接收者类型 同上
    phead->srcid = dstid;   //16位无符号整型，当客户端向游戏服务器发送消息时ScrID为SessionID
    phead->dstid = srcid;   //16位无符号整型，当游戏服务器向客户端发送消息是DstID为SessionID

    // printf("sendmsg srcid = %d\n", phead->srcid);


    if (map.fnd(&index, dstid) < 0)
    {
        fprintf(stderr, "Err: CDBEngine map.fnd() return neg val !!\n" );
		fprintf(stderr, "Err: CDBEngine dstid = %d !!\n", dstid );
        return -1;
    }

    // printf("CDBEngine::    msglen = %d \n", phead->msglen);
    if (pqm->pushmsg(index, &tmpmsgbuf) < 0)
    {
        fprintf(stderr, "Err: CDBEngine pqm->pushmsg() return neg val !!!\n");
        return -1;
    }
    return 0;
}















/* int CDBEngine::handle(StMsgBuffer *pmsg, CShmQueueMulti *pshmqm)
{
    CMsgHead *phead = (CMsgHead *)pmsg;
    uint16_t srcid = phead->srcid;
    uint16_t dstid = phead->dstid;
    switch (phead->msgid)
    {
    case MSGID_I2M_NEW_CONNECT:
        if (map.reg(srcid, dstid) < 0)
        {
            fprintf(stderr, "Err: map.reg() return neg val !!!\n");
        }
        break;

    case MSGID_I2M_CLO_CONNECT:
        if (map.del(srcid) < 0)
        {
            fprintf(stderr, "Err: map.del() return neg val !!\n");
        }
        break;

    case MSGID_I2M_LOGIN:
        return handle_MSGID_I2M_LOGIN(pmsg, pshmqm);
        break;

    default:
        return -1;
        break;
    }
    return 0;
}


int CDBEngine::handle_MSGID_I2M_LOGIN(StMsgBuffer *pmsg, CShmQueueMulti *pshmqm)
{
    StMsgBuffer tmpmsgbuf;
    tmpmsgbuf.n = 0;

    int index;

    CMsgHead *phead = (CMsgHead *)pmsg;
    // uint16_t srcid = phead->srcid;
    uint16_t dstid = phead->dstid;

    CMsgRequestLoginPara *pinpara =
        (CMsgRequestLoginPara *)(pmsg->buf + sizeof(CMsgHead));
    CMsgResponseLoginPara *poutpara =
        (CMsgResponseLoginPara *)(tmpmsgbuf.buf + sizeof(CMsgHead));
    // inpara.buf2para(pmsg->buf + sizeof(CMsgHead));
    // pinpara->print();

    poutpara->m_unUin = 1;
    poutpara->m_unSessionID = 2;
    poutpara->m_bResultID = 3;
    strcpy(poutpara->m_stPlayerInfo.m_szUserName, pinpara->username);
    // printf("pinpara->username %s\n", pinpara->username);
    poutpara->m_stPlayerInfo.m_unUin = 1;
    poutpara->m_stPlayerInfo.m_bySex = 0;
    poutpara->m_stPlayerInfo.m_unLevel = 99;
    poutpara->m_stPlayerInfo.m_unWin = 90;
    poutpara->m_stPlayerInfo.m_unLose = 8;
    poutpara->m_stPlayerInfo.m_unRun = 1;

    phead = (CMsgHead *)tmpmsgbuf.buf;
    phead->msglen = sizeof(CMsgHead) + sizeof(CMsgResponseLoginPara);
    // printf("phead->msglen = %d\n", phead->msglen);
    phead->msgid = MSGID_I2M_LOGIN; //16位无符号整型，消息ID
    phead->msgtype = Response;   //16位无符号整型，消息类型，当前主要有Requst、Response以及Notify三种类型
    phead->msgseq = 1234567890;     //32位无符号整型，消息序列号
    phead->srcfe = FE_GAMESVRD ;       //8位无符号整型，消息发送者类型，当前主要有FE_CLIENT、FE_GAMESVRD以及FE_DBSVRD三种
    phead->dstfe = FE_CLIENT;     //8位无符号整型，消息接收者类型 同上
    phead->srcid = dstid;   //16位无符号整型，当客户端向游戏服务器发送消息时ScrID为SessionID
    phead->dstid = 0;   //16位无符号整型，当游戏服务器向客户端发送消息是DstID为SessionID


    if (map.fnd(&index, dstid) < 0)
    {
        fprintf(stderr, "Err: map.fnd() return neg val !!\n" );
        return -1;
    }

    // printf("CDBEngine::    msglen = %d \n", phead->msglen);
    if (pshmqm->pushmsg(index, &tmpmsgbuf) < 0)
    {
        fprintf(stderr, "Err: pshmqm->pushmsg() return neg val !!!\n");
        return -1;
    }
    return 0;
} */