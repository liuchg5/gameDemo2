#include "msgcommon.h"

void set_MSGID_I2M_NEW_CONNECT(StMsgBuffer *pmsgbuf, int index, int srvfd)
{
    CMsgHead *phead = (CMsgHead *)pmsgbuf->buf;
    phead->msglen = sizeof(CMsgHead);
    phead->msgid = MSGID_I2M_NEW_CONNECT;
    phead->msgtype = 0;
    phead->msgseq = 0;
    phead->srcfe = 0;
    phead->dstfe = 0;
    phead->srcid = index;
    phead->dstid = srvfd;

}


void set_MSGID_I2M_CLO_CONNECT(StMsgBuffer *pmsgbuf, int index, int srvfd)
{
    CMsgHead *phead = (CMsgHead *)pmsgbuf->buf;
    phead->msglen = sizeof(CMsgHead);
    phead->msgid = MSGID_I2M_CLO_CONNECT;
    phead->msgtype = 0;
    phead->msgseq = 0;
    phead->srcfe = 0;
    phead->dstfe = 0;
    phead->srcid = index;
    phead->dstid = srvfd;
}