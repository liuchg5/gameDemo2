#ifndef CGMENGINE_H
#define CGMENGINE_H

#include "../msg/StMsgBuffer.h"
#include "../msg/CMsgPara.h"
#include "../msg/CMsgHead.h"
#include "../queue/CShmQueueMulti.h"
#include "../queue/CShmQueueSingle.h"
#include "../common/CMap.h"


class CGMEngine
{
public:
	int EPOLL_SIZE;
	CMap map;

	CGMEngine(int epoll_size);
	~CGMEngine();
	
	// int handle(StMsgBuffer * pinmsg, CShmQueueMulti * pshmqm, CShmQueueSingle * pshmqs);
	// int handle_MSGID_I2M_LOGIN(StMsgBuffer * pinmsg, CShmQueueMulti * pshmqm, CShmQueueSingle * pshmqs);
	int handle_client(StMsgBuffer * pmsg, CShmQueueMulti * pqm, CShmQueueSingle * pqs);
	int handle_db(StMsgBuffer * pmsg, CShmQueueMulti * pqm, CShmQueueSingle * pqs);


	int handle_RequestLogin(StMsgBuffer * pmsg, CShmQueueMulti * pqm, CShmQueueSingle * pqs);
	int handle_RequestUserInfo(StMsgBuffer * pmsg, CShmQueueMulti * pqm, CShmQueueSingle * pqs);


	 int handle_debug(StMsgBuffer * pinmsg, CShmQueueMulti * pshmqm);  //debug
	 int handle_MSGID_I2M_LOGIN(StMsgBuffer * pinmsg, CShmQueueMulti * pshmqm);//debug
};

#endif
