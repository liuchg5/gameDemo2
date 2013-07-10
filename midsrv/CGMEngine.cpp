#include "CGMEngine.h"




CGMEngine::CGMEngine(int epoll_size):
    EPOLL_SIZE(epoll_size),
    map(epoll_size)
{

}
CGMEngine::~CGMEngine()
{

}

int CGMEngine::handle_client(StMsgBuffer * pmsg, CShmQueueMulti * pqm, CShmQueueSingle * pqs)
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

    case MSGID_REQUESTLOGIN:
        return handle_RequestLogin(pmsg, pqm, pqs);
        break;
		
    default:
        return -1;
        break;
    }
    return 0;
}


int CGMEngine::handle_db(StMsgBuffer * pmsg, CShmQueueMulti * pqm, CShmQueueSingle * pqs)
{
    CMsgHead *phead = (CMsgHead *)pmsg;
	
	switch (phead->msgid)
    {
	case MSGID_REQUESTUSERINFO:
        return handle_RequestUserInfo(pmsg, pqm, pqs);
        break;

    default:
        return -1;
        break;
    }
    return 0;
}

int CGMEngine::handle_RequestLogin(StMsgBuffer * pmsg, CShmQueueMulti * pqm, CShmQueueSingle * pqs)
{
    StMsgBuffer tmpmsgbuf;
    tmpmsgbuf.n = 0;

    CMsgHead *phead = (CMsgHead *)pmsg->buf;
    // uint16_t srcid = phead->srcid;
    uint16_t dstid = phead->dstid;

    CMsgRequestLoginPara *pinpara =
        (CMsgRequestLoginPara *)(pmsg->buf + sizeof(CMsgHead));
    CRequestUserInfoPara *poutpara =
        (CRequestUserInfoPara *)(tmpmsgbuf.buf + sizeof(CMsgHead));
    // inpara.buf2para(pmsg->buf + sizeof(CMsgHead));
    
	pinpara->print(); // debug

    strcpy(poutpara->m_szUserName, pinpara->username);
    // printf("pinpara->username %s\n", pinpara->username);


    phead = (CMsgHead *)tmpmsgbuf.buf;
    phead->msglen = sizeof(CMsgHead) + sizeof(CMsgResponseLoginPara);
    // printf("phead->msglen = %d\n", phead->msglen);
    phead->msgid = MSGID_REQUESTUSERINFO; //16位无符号整型，消息ID
    phead->msgtype = Request;   //16位无符号整型，消息类型，当前主要有Request、Response以及Notify三种类型
    phead->msgseq = 1234567890;     //32位无符号整型，消息序列号
    phead->srcfe = FE_GAMESVRD ;       //8位无符号整型，消息发送者类型，当前主要有FE_CLIENT、FE_GAMESVRD以及FE_DBSVRD三种
    phead->dstfe = FE_DBSVRD;     //8位无符号整型，消息接收者类型 同上
    phead->srcid = dstid;   //16位无符号整型，当客户端向游戏服务器发送消息时ScrID为SessionID
    phead->dstid = 0;   //16位无符号整型，当游戏服务器向客户端发送消息是DstID为SessionID


    // printf("CGMEngine::    msglen = %d \n", phead->msglen);
    if (pqs->pushmsg(&tmpmsgbuf) < 0)
    {
        fprintf(stderr, "Err: pqs->pushmsg() return neg val mean it is full !!!\n");
        return -1;
    }
    return 0;
}

	
int CGMEngine::handle_RequestUserInfo(StMsgBuffer * pmsg, CShmQueueMulti * pqm, CShmQueueSingle * pqs)
{
   StMsgBuffer tmpmsgbuf;
    tmpmsgbuf.n = 0;

    int index;

    CMsgHead *phead = (CMsgHead *)pmsg;
    // uint16_t srcid = phead->srcid;
    uint16_t dstid = phead->dstid;

    CResponseUserInfoPara *pinpara =
        (CResponseUserInfoPara *)(pmsg->buf + sizeof(CMsgHead));
    CMsgResponseLoginPara *poutpara =
        (CMsgResponseLoginPara *)(tmpmsgbuf.buf + sizeof(CMsgHead));
    // inpara.buf2para(pmsg->buf + sizeof(CMsgHead));
    
	pinpara->print(); //debug

    poutpara->m_unUin = pinpara->m_stPlayerInfo.m_unUin;
    poutpara->m_unSessionID = phead->dstid;
    poutpara->m_bResultID = 0;
	memcpy(&poutpara->m_stPlayerInfo, &pinpara->m_stPlayerInfo, sizeof(m_stPlayerInfo));


    phead = (CMsgHead *)tmpmsgbuf.buf;
    phead->msglen = sizeof(CMsgHead) + sizeof(CMsgResponseLoginPara);
    // printf("phead->msglen = %d\n", phead->msglen);
    phead->msgid = MSGID_REQUESTLOGIN; //16位无符号整型，消息ID
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

    // printf("CGMEngine::    msglen = %d \n", phead->msglen);
    if (pqm->pushmsg(index, &tmpmsgbuf) < 0)
    {
        fprintf(stderr, "Err: pqm->pushmsg() return neg val !!!\n");
        return -1;
    }
    return 0;
}















/* int CGMEngine::handle(StMsgBuffer *pmsg, CShmQueueMulti *pshmqm)
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


int CGMEngine::handle_MSGID_I2M_LOGIN(StMsgBuffer *pmsg, CShmQueueMulti *pshmqm)
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

    // printf("CGMEngine::    msglen = %d \n", phead->msglen);
    if (pshmqm->pushmsg(index, &tmpmsgbuf) < 0)
    {
        fprintf(stderr, "Err: pshmqm->pushmsg() return neg val !!!\n");
        return -1;
    }
    return 0;
} */