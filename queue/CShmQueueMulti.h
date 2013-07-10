#ifndef CSHMQUEUEMULTIMULTI_H
#define CSHMQUEUEMULTIMULTI_H

#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <netinet/in.h>

#include "CQueue.h"

#include "../msg/StMsgBuffer.h"
#include "../socket/CSocketList.h"
#include "../common/CStatistics.h"

// queue中的index从0开始，而list中是1开始，注意，外部使用1开始的index
class CShmQueueMulti  // 只能使用指针模式，指向共享内存区基址
{
public:
    int SHM_SIZE;
    int FTOLK_VAL;
    int shmid;
    char *base;

    int EPOLL_SIZE;
    int size;
    char *pmsgbase;  // 通道N，每次使用要根据index来重赋值
    CQueue *pq;  // 通道N，每次使用要根据index来重赋值

    CStatistics sta;

public:
    CShmQueueMulti();
    ~CShmQueueMulti();

    int crt(int size, int ftolk_val);  // 创建共享内存区
    int del();
    int get();  // 获取共享内存区基址
    int det();

    void init(int epoll_size);  // 建立队列在共享内存区上的映射
    void clear();
    void clear(int index);

    int pushmsg(int index, StMsgBuffer *pmsg)   // index内部需要-1处理
    {
        sta.check("CShmQueueMulti.pushmsg");
        index--;
        pq = (CQueue *)(base + index * size);
        pmsgbase = (char *)pq + sizeof(CQueue);
        return pq->push(pmsgbase, pmsg->buf);
    }
    int popmsg_complex(int epfd, CSocketList *plist);  // 返回修改的fd数目

};

#endif
