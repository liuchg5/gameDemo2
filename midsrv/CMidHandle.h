#ifndef CMIDHANDLE_H
#define CMIDHANDLE_H

#include "../msg/StMsgBuffer.h"
#include "../msg/CMsgPara.h"
#include "../msg/CMsgHead.h"
#include "../queue/CShmQueueMulti.h"
#include "../queue/CShmQueueSingle.h"
#include "../common/CMap.h"


class CMidHandle
{
public:
	int EPOLL_SIZE;
	CMap map;

	CMidHandle(int epoll_size);
	~CMidHandle();
	
	// int handle(StMsgBuffer * pinmsg, CShmQueueMulti * pshmqm, CShmQueueSingle * pshmqs);
	// int handle_MSGID_I2M_LOGIN(StMsgBuffer * pinmsg, CShmQueueMulti * pshmqm, CShmQueueSingle * pshmqs);



	int handle(StMsgBuffer * pinmsg, CShmQueueMulti * pshmqm);
	int handle_MSGID_I2M_LOGIN(StMsgBuffer * pinmsg, CShmQueueMulti * pshmqm);
};

#endif
