#ifndef CDBENGINE_H
#define CDBENGINE_H

#include "../msg/StMsgBuffer.h"
#include "../msg/CMsgPara.h"
#include "../msg/CMsgHead.h"
#include "../queue/CShmQueueMulti.h"
#include "../queue/CShmQueueSingle.h"
#include "../common/CMap.h"


class CDBEngine
{
public:
	int EPOLL_SIZE;
	CMap map;

	CDBEngine(int epoll_size);
	~CDBEngine();

	int handle(StMsgBuffer * pmsg, CShmQueueMulti * pqm);

private:
	int handle_RequestUserInfo(StMsgBuffer * pmsg, CShmQueueMulti * pqm);
};

#endif
