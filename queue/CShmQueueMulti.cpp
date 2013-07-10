

#include "CShmQueueMulti.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <sys/ipc.h>
#include <sys/shm.h>

CShmQueueMulti::CShmQueueMulti():
    shmid(-1),
    base(NULL),
    EPOLL_SIZE(0),
    size(0),
    pmsgbase(NULL),
    pq(NULL)
{

}


CShmQueueMulti::~CShmQueueMulti()
{

}

int CShmQueueMulti::crt(int size, int ftolk_val)
{
    SHM_SIZE = size;
    FTOLK_VAL = ftolk_val;
    shmid = shmget(FTOLK_VAL, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0)
    {
        fprintf(stderr, "Err: CShmQueueMulti create shared memory \n");
        fprintf(stderr, "Err: CShmQueueMulti errno = %d (%s) \n", errno, strerror(errno));
        return shmid;
    }
    fprintf(stdout, "Info: CShmQueueMulti create shared memory \n");
    fprintf(stdout, "Info: CShmQueueMulti shmid = %d \n", shmid);
    return shmid;
}

int CShmQueueMulti::del()
{
    int ret = shmctl( shmid, IPC_RMID, 0 );
    if ( ret == 0 )
        fprintf(stdout, "Info: CShmQueueMulti Shared memory removed \n" );
    else
        fprintf(stderr, "Err: CShmQueueMulti Shared memory remove failed \n" );
    base = NULL;
    return ret;
}

int CShmQueueMulti::get()
{
    if (shmid >= 0)
    {
        base = (char *)shmat(shmid, 0, 0);
        // if (base == (char *) - 1)
        // fprintf(stderr, "Err: CShmQueueMulti base == -1, errno = %d   \n", errno);
        // fprintf(stderr, "Err: CShmQueueMulti get(), base = %x  \n", base);
    }
    return 0;
}
int CShmQueueMulti::det()
{
    shmdt(base);
    base = 0;
    return 0;
}










void CShmQueueMulti::init(int epoll_size)
{
    EPOLL_SIZE = epoll_size;
    size = SHM_SIZE / epoll_size;
    for (int i = 0; i < EPOLL_SIZE; ++i)
    {
        pq = (CQueue *)(base + i * size);
        pmsgbase = (char *)pq + sizeof(CQueue);
        pq->CQueue_Construction(size - sizeof(CQueue));
    }
}
void CShmQueueMulti::clear()
{
    for (int i = 0; i < EPOLL_SIZE; ++i)
    {
        pq = (CQueue *)(base + i * size);
        pq->clear();
    }
}
void CShmQueueMulti::clear(int index)
{
    index--;
    pq = (CQueue *)(base + index * size);
    pmsgbase = (char *)pq + sizeof(CQueue);
    pq->clear();
}



// int CShmQueueMulti::pushmsg(int index, StMsgBuffer *pmsg)
// {
//     sta.check("CShmQueueMulti.pushmsg");

//     index--;
//     pq = (CQueue *)(base + index * size);
//     pmsgbase = (char *)pq + sizeof(CQueue);
//     return pq->push(pmsgbase, pmsg->buf);
// }

int CShmQueueMulti::popmsg_complex(int epfd, CSocketList *plist)
{
    // index的同步走msg的方式，这里假设就是一一对应
    struct epoll_event ev;
    int n = 0;
    for (int i = 0; i < EPOLL_SIZE; ++i)
    {
        // 取出对应的值
        pq = (CQueue *)(base + i * size);
        pmsgbase = (char *)pq + sizeof(CQueue);
        // 判断通道是否为空
        if (pq->top_just(pmsgbase) <= 0)
            continue;

        // 判断socketinfo是否还可以写入
        if (plist->val[i + 1].sendflag == 1)
        {
            fprintf(stderr, "Err: CShmQueueMulti::pop_complex() sendflag is 1 !!!!\n");
            continue;
        }
        //
        pq->pop(pmsgbase, plist->val[i + 1].sendmsg.buf);
        plist->val[i + 1].sendmsg.n = 0;
        plist->val[i + 1].sendflag = 1;
        //
        ev.data.fd = plist->val[i + 1].srvfd;
        ev.events = EPOLLOUT;
        if (epoll_ctl(epfd, EPOLL_CTL_MOD, ev.data.fd, &ev) < 0)
        {
            fprintf(stderr, "Err: popmsg_complex epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) \n ");
            fprintf(stderr, "Err: popmsg_complex errno = %d (%s) \n ", errno, strerror(errno));
            return -1;
        }
        //
        n++;
    }
    return n;
}
