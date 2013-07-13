

#include "CShmQueueMulti.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include "../socket/CSocketSrvEpoll.h"


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
        fprintf(stderr, "Err: CShmQueueMulti SHM_SIZE = %d MB \n", SHM_SIZE / 1024 / 1024);
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
    EPOLL_SIZE = epoll_size + 1; // 0 不用
    size = SHM_SIZE / EPOLL_SIZE;
    for (int i = 1; i < EPOLL_SIZE; ++i)
    {
        pq = (CQueue *)(base + i * size);
        pmsgbase = (char *)pq + sizeof(CQueue);
        pq->CQueue_Construction(size - sizeof(CQueue));
    }
}
void CShmQueueMulti::clear()
{
    for (int i = 1; i < EPOLL_SIZE; ++i)
    {
        pq = (CQueue *)(base + i * size);
        pq->clear();
    }
}
void CShmQueueMulti::clear(int index)
{
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

int CShmQueueMulti::popmsg_complex(CSocketSrvEpoll *psrv)
{
    // index的同步走msg的方式，这里假设就是一一对应
    for (int i = 1; i < EPOLL_SIZE; ++i)
    {
        

        // 轮询方式
        if (psrv->socketlist.val[i].writable == 0)
            continue;

        // 取出对应的值
        pq = (CQueue *)(base + i * size);
        if (pq->empty() == 1)
            continue;
            
        // printf("pq->empty() = %d \n", pq->empty());

        CSocketList *plist = &(psrv->socketlist);
        CSocketInfo *psi = &(plist->val[i]);

        

        // 判断srvfd是否-1  !!!!
        if (plist->val[i].srvfd < 0)
        {
            clear(i);
            continue;
        }

        

        // 取出对应的值
        // pq = (CQueue *)(base + i * size);
        pmsgbase = (char *)pq + sizeof(CQueue);
        StMsgBuffer *psmsg = &(psi->sendmsg);
        int socketfd = psi->srvfd;
        int index = i;
        StMsgBuffer tmpmsgbuf;
        // // 判断是否有数据
        // if (pq->empty(pmsgbase))
        //     continue;

        if (psi->sendflag == 0)  // 没有东西要发送
        {
            if (pq->pop(pmsgbase, psmsg->buf) > 0)
            {
                psmsg->n = 0;
                psi->sendflag = 1; // 继续发送
            }
            else // 通道内没数据
            {
                psmsg->n = 0;  // 复位
                psi->sendflag = 0; // 可以接受新的消息
            }
        }
        while (psi->sendflag == 1)
        {
            uint32_t msglen = *(uint32_t *)psmsg->buf;
            ssize_t n = send(socketfd, psmsg->buf + psmsg->n, msglen - psmsg->n, 0);
            if (n < 0)
            {
                if (errno == EWOULDBLOCK || errno == EAGAIN)   // errno == EINTR ||  没有发送完
                {
                    fprintf(stderr, "Err: CSocketSrvEpoll send() is waiting !!! \n");
#ifdef ERR_EPOLL_SEND_WAIT
                    return ERR_EPOLL_SEND_WAIT;
#endif
                    psi->writable = 0; // 不可写
                    break;//
                }
                else     // 出错
                {
                    fprintf(stderr, "Err: CSocketSrvEpoll send() error happen \n");
                    fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));
                    psrv->myclose(index);
                    // TODO: 通知mainsrv
                    set_MSGID_I2M_CLO_CONNECT(&tmpmsgbuf, index, socketfd);
                    if (psrv->pqs->pushmsg(&tmpmsgbuf) <= 0)
                    {
                        fprintf(stderr, "Err: CSocketSrvEpoll close connect msg cannot push into queue !!!!!!\n ");
                        return -1;
                    }
                    break;
                }
            }
            else if (n == 0)    // 一般不返回0
            {
                fprintf(stderr, "Err: CSocketSrvEpoll send() n == 0 \n");
                break;
            }
            else
            {
                psmsg->n += n;
                if (psmsg->n == msglen)  // 发送完了，还需要判断通道内是否有数据还要发送
                {
                    if (pq->pop(pmsgbase, psmsg->buf) > 0)
                    {
                        psmsg->n = 0;
                        psi->sendflag = 1; // 继续发送
                    }
                    else // 通道内没数据
                    {
                        psmsg->n = 0;  // 复位
                        psi->sendflag = 0; // 可以接受新的消息
                    }
                }
            }
        } // end while ()
    }// end for ()
    return 0;
}



int CShmQueueMulti::popmsg_complex_noset_epollout(int epfd, CSocketList *plist)
{
    // index的同步走msg的方式，这里假设就是一一对应
    // struct epoll_event ev;
    int n = 0;
    StMsgBuffer msgbuf;
    for (int i = 1; i < EPOLL_SIZE; ++i)
    {
        // 取出对应的值
        pq = (CQueue *)(base + i * size);
        pmsgbase = (char *)pq + sizeof(CQueue);

        // 清空通道
        while ( (pq->pop(pmsgbase, msgbuf.buf)) > 0 )
        {

        }

        n++;
    }
    return n;
}
