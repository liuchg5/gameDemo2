#ifndef MSGCOMMON_H
#define MSGCOMMON_H

#include "../msg/StMsgBuffer.h"
#include "../msg/CMsgHead.h"
#include "../msg/CMsgPara.h"


void set_MSGID_I2M_NEW_CONNECT(StMsgBuffer * pmsgbuf, int index, int srvfd);

void set_MSGID_I2M_CLO_CONNECT(StMsgBuffer * pmsgbuf, int index, int srvfd);

#endif
