#ifndef CSHMQUEUESINGLESINGLE_H
#define CSHMQUEUESINGLESINGLE_H

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <sys/ipc.h>
#include <sys/shm.h>


#include "CQueue.h"

#include "../msg/StMsgBuffer.h"
#include "../common/CStatistics.h"


class CShmQueueSingle  // 只能使用指针模式，指向共享内存区基址
{
public:
    int SHM_SIZE;
    int FTOLK_VAL;
    int shmid;
    char *base;

    char *pmsgbase;  // 通道1
    CQueue *pq;

    CStatistics sta;

public:
    CShmQueueSingle();
    ~CShmQueueSingle();

    int crt(int size, int ftolk_val);  // 创建共享内存区
    int del();
    int get();  // 获取共享内存区基址
    int det();

    void init();  // 建立队列在共享内存区上的映射
    void clear()
    {
        pq->clear();
    }

    int full()
    {
        return pq->full();
    }
    int empty()
    {
        return pq->empty();
    }
    int pushmsg(StMsgBuffer *pmsg)   // 只解开msglen 返回压入字符长度表示成功，返回-1表示失败
    {
        sta.check("CShmQueueSingle.pushmsg");
        return pq->push(pmsgbase, pmsg->buf);
    }
    int popmsg(StMsgBuffer *pmsg)
    {
        return pq->pop(pmsgbase, pmsg->buf);
    }
    int popmsg_just()
    {
        return pq->pop_just(pmsgbase);
    }
    int topmsg(StMsgBuffer *pmsg)
    {
        return pq->top(pmsgbase, pmsg->buf);
    }
};

#endif
