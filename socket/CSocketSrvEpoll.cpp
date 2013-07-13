#include "CSocketSrvEpoll.h"

#include "CSocketInfo.h"

#include "../common/msgcommon.h"

CSocketSrvEpoll::CSocketSrvEpoll(int epoll_size, int epoll_timeout, int listenq, CShmQueueSingle * vpqs, CShmQueueMulti * vpqm):
    PORT_NUMBER(10203),
    EPOLL_SIZE(epoll_size),
    EPOLL_TIMEOUT(epoll_timeout),
    LISTENQ(listenq),
    epfd(-1),
    listenfd(-1),
    clientAddrLen(0),
    socketlist(epoll_size),
    pqs(vpqs),
    pqm(vpqm)
{
    events = new struct epoll_event[EPOLL_SIZE + 1];
    if (events == NULL)
    {
        fprintf(stderr, "Err: CSocketSrvEpoll new struct epoll_event[EPOLL_SIZE + 1] \n ");
        fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));
    }

    memset(&serveraddr, 0, sizeof(struct sockaddr_in));
    memset(&clientaddr, 0, sizeof(struct sockaddr_in));
}

CSocketSrvEpoll::~CSocketSrvEpoll()
{
    delete [] events;
    events = NULL;
}

int CSocketSrvEpoll::open(const char *serv_addr, int port_number)
{
    int ret;
    // 创建epoll
    epfd = epoll_create(EPOLL_SIZE);
    if (epfd < 0)
    {
        fprintf(stderr, "Err: CSocketSrvEpoll epoll_create() \n ");
        fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));
        return epfd;
    }

    // 打开监听的socket描述符
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        fprintf(stderr, "Err: CSocketSrvEpoll socket(AF_INET, SOCK_STREAM, 0) \n ");
        fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));
        return listenfd;
    }

    // 设为非阻塞
    if (setnonblocking(listenfd) != 0)
    {
        fprintf(stderr, "Err: CSocketSrvEpoll setnonblocking() \n ");
        fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));
    }

    // 注册epoll事件
    ev.data.fd = listenfd;
    ev.events = EPOLLIN ;//响应读事件
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
    if (ret < 0)
    {
        fprintf(stderr, "Err: CSocketSrvEpoll epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev) \n ");
        fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));
        return ret;
    }

    // 注册socketlist
    socketlist.val[0].srvfd = listenfd;

    // 处理服务器地址
    serveraddr.sin_family = AF_INET;
    PORT_NUMBER = port_number;
    inet_aton(serv_addr, &(serveraddr.sin_addr));
    serveraddr.sin_port = htons(PORT_NUMBER);

    // 将socket描述符和服务器地址端口绑定
    ret = bind(listenfd, (sockaddr *) &serveraddr, sizeof(serveraddr));
    if (ret < 0)
    {
        fprintf(stderr, "Err: CSocketSrvEpoll bind(listenfd, (sockaddr *) &serveraddr, sizeof(serveraddr)) \n ");
        fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n", errno, strerror(errno));
        return ret;
    }

    // 开始监听
    ret = listen(listenfd, LISTENQ);
    if (ret < 0)
    {
        fprintf(stderr, "Err: CSocketSrvEpoll listen(listenfd, LISTENQ) \n ");
        fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));
        return ret;
    }

    return 0; // success
}

int CSocketSrvEpoll::myclose()
{
    // 需要关闭所有打开的socket
    for (int i = 1; i < EPOLL_SIZE + 1; i++)
    {
        if (socketlist.val[i].srvfd > 0)
        {
            close(socketlist.val[i].srvfd);
            // TODO: 通知mainsrv
        }
    }
    // 关闭监听的socket
    close(socketlist.val[0].srvfd);
    socketlist.clear();

    return 0;
}

int CSocketSrvEpoll::myclose(int index) 
{

        if (socketlist.val[index].srvfd > 0)
        {
            close(socketlist.val[index].srvfd);
            // TODO: 通知mainsrv
        }

    socketlist.erase(index);

    return 0;
}

// #define RECV_WAIT_TIME 1000
int CSocketSrvEpoll::my_epoll_wait() // just read and set
{
    int index;
    int socketfd;
    StMsgBuffer tmpmsgbuf;
	
	static long recv_wait_time = 0;//debug
	
 
    int nfds = epoll_wait(epfd, events, EPOLL_SIZE, EPOLL_TIMEOUT);
    if ( nfds < 0 )
    {
        fprintf(stderr, "Err: CSocketSrvEpoll epoll_wait() \n ");
        fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));// system interupter ?
        // return -1;  // for system interrupt so donot return
    }

    for (int i = 0; i < nfds; ++i)
    {
        socketfd = events[i].data.fd;  // 获取fd
        index = socketlist.find(socketfd); // 获取上下文的下标

        if (index < 0)
        {
            fprintf(stderr, "Err: CSocketSrvEpoll cannot find socketfd=>socketlist.val[] \n ");
            return -1;
        }
        CSocketInfo *psi = &(socketlist.val[index]);  // 获取上下文指针
        StMsgBuffer *prmsg = &(psi->recvmsg);
        StMsgBuffer *psmsg = &(psi->sendmsg);
        // int recvflag = psi->recvflag;
        // int sendflag = psi->sendflag;


        //-------------------- 一个新socket用户连接到监听端口，建立新的连接
        if (socketfd == listenfd)
        {
            // fprintf(stdout, "Info: CSocketSrvEpoll new conncet. \n");
            // 建立新连接
            clientAddrLen =  sizeof(struct sockaddr_in);
			int connfd;
            
			// while ((connfd = accept(listenfd, (sockaddr *) &clientaddr, &clientAddrLen )) > 0)
			// {
            connfd = accept(listenfd, (sockaddr *) &clientaddr, &clientAddrLen );
			if (connfd < 0)
            {
                fprintf(stderr, "Err: CSocketSrvEpoll accept new connect failed. \n");
                fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));
                return -1; 
				continue;
            }
			
            // 设为非阻塞
            if (setnonblocking(connfd) != 0)
            {
                fprintf(stderr, "Err: CSocketSrvEpoll setnonblocking(connfd) \n ");
                fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));
				return -1;
            }

            // 新连接的上下文
            index = socketlist.idle();
            if (index < 0)
            {
                fprintf(stderr, "Err: CSocketSrvEpoll socketlist is full!!! \n");
				return -1;
            }
            socketlist.val[index].srvfd = connfd;

            // 注册新的连接
            ev.data.fd = connfd; // 找到空闲的socketinfo
            ev.events = EPOLLIN | EPOLLOUT | EPOLLET;

            printf("Info: new connect fd = %d, index = %d \n", connfd, index);

            if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) < 0)
            {
                fprintf(stderr, "Err: CSocketSrvEpoll epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) \n ");
                fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));
                return -1;
				//continue;
            }

            set_MSGID_I2M_NEW_CONNECT(&tmpmsgbuf, index, connfd);
            if (pqs->pushmsg(&tmpmsgbuf) <= 0)
            {
                fprintf(stderr, "Err: CSocketSrvEpoll new connect msg cannot push, queue full !!!!!!\n ");
				return -1;
            }
            
			// } // end while ( accept() )
			continue;
		}

        
// ------------------- 读取数据
        if (events[i].events & EPOLLIN)
        {

            if (psi->recvflag == 1)  // 已经收取了一个msg，还未取走
            {
                // TODO 压入共享内存区
                CMsgHead *phead = (CMsgHead *)prmsg;
                phead->dstid = socketfd; // read需修改dstid
                if (pqs->pushmsg(prmsg) >= 0)
                {
                    prmsg->n = 0;
                    psi->recvflag = 0;
                }
                else
                {
                    fprintf(stderr, "Err: CSocketSrvEpoll EPOLLIN but queue is full < 0 !!!!!!\n ");
					return -1;
                }
            }
            while (psi->recvflag == 0)
            {
                
                if (prmsg->n < 4)  // 还未获取到msglen
                {
                    ssize_t n = recv(socketfd, prmsg->buf + prmsg->n, 4 - prmsg->n, 0); //
                    if (n > 0)
                    {
                        prmsg->n += n;
                    }
                    else if (n < 0)
                    {
                        if (errno == EWOULDBLOCK || errno == EAGAIN)  // errno == EINTR ||  跳出，等待下次接收
                        {
                            #ifdef RECV_WAIT_TIME
							if (recv_wait_time++ > RECV_WAIT_TIME){
								fprintf(stderr, "Err:CSocketSrvEpoll recv wait %d times !! \n", RECV_WAIT_TIME);
								recv_wait_time = 0;
							}
                            #endif
                            break;// 
                        }
                        else if (errno == ECONNRESET)     // 对方关闭了socket
                        {
                            fprintf(stdout, "Warn: CSocketSrvEpoll ECONNRESET \n ");
                            myclose(index);
                            // TODO: 通知mainsrv
                            set_MSGID_I2M_CLO_CONNECT(&tmpmsgbuf, index, socketfd);
                            if (pqs->pushmsg(&tmpmsgbuf) <= 0)
                            {
                                fprintf(stderr, "Err: CSocketSrvEpoll close connect msg cannot push into queue, full !!!!!!\n ");
								return -1;
                            }
                            break;
                        }
                        else
                        {
                            fprintf(stdout, "Warn: CSocketSrvEpoll n < 0 but donot know why \n ");
                            myclose(index);
                            // TODO: 通知mainsrv
                            set_MSGID_I2M_CLO_CONNECT(&tmpmsgbuf, index, socketfd);
                            if (pqs->pushmsg(&tmpmsgbuf) <= 0)
                            {
                                fprintf(stderr, "Err: CSocketSrvEpoll close connect msg cannot push into queue, full !!!!!!\n ");
								return -1;
                            }
                            break;
                        }
                    }
                    else if (n == 0)    // 对端关闭
                    {
                        fprintf(stdout, "Warn: CSocketSrvEpoll recv n = 0 \n");
                        myclose(index);
                        // TODO: 通知mainsrv
                        set_MSGID_I2M_CLO_CONNECT(&tmpmsgbuf, index, socketfd);
                        if (pqs->pushmsg(&tmpmsgbuf) <= 0)
                        {
                            fprintf(stderr, "Err: CSocketSrvEpoll close connect msg cannot push into queue, full !!!!!!\n ");
							return -1;
                        }
                        break;
                    }
                }
                if (prmsg->n >= 4)  // 已经获取到msglen
                {
                    uint32_t msglen = *(uint32_t *)prmsg->buf;
                    ssize_t n = recv(socketfd, prmsg->buf + prmsg->n, msglen - prmsg->n, 0); // 保证读取的是一个msg
                    if (n > 0)
                    {
                        prmsg->n += n;
                        if (prmsg->n == msglen)  // 收取了一个完整msg
                        {
                            CMsgHead *phead = (CMsgHead *)prmsg;
                            phead->dstid = socketfd; // read需修改dstid
                            // TODO 压入共享内存区
                            if (pqs->pushmsg(prmsg) >= 0)
                            {
                                prmsg->n = 0;
                                psi->recvflag = 0;
                            }
                            else
                            {
                                fprintf(stderr, "Err: CSocketSrvEpoll pushmsg() failed because it's full !!!!\n");
								return -1;
                                // 如果压入共享内存区失败，则先保留在socketinfo的msg缓冲区
                                psi->recvflag = 1; //只是置标志位而已，TODO 这里有问题，应该通过ShmQueue来取走
                            }
                        }
                    }
                    if (n < 0)
                    {
                        if (errno == EWOULDBLOCK || errno == EAGAIN)  // errno == EINTR ||  跳出，等待下次接收
                        {
                            #ifdef RECV_WAIT_TIME
							if (recv_wait_time++ > RECV_WAIT_TIME){
								fprintf(stderr, "Err:CSocketSrvEpoll recv wait %d times !! \n", RECV_WAIT_TIME);
								recv_wait_time = 0;
							}
                            #endif
                            break;
                        }
                        else if (errno == ECONNRESET)     // 对方关闭了socket
                        {
                            fprintf(stdout, "Warn: CSocketSrvEpoll ECONNRESET \n");
                            myclose(index);
                            // TODO: 通知mainsrv
                            set_MSGID_I2M_CLO_CONNECT(&tmpmsgbuf, index, socketfd);
                            if (pqs->pushmsg(&tmpmsgbuf) <= 0)
                            {
                                fprintf(stderr, "Err: CSocketSrvEpoll close connect msg cannot push into queue !!!!!!\n ");
								return -1;
                            }
                            break;
                        }
                        else
                        {
                            fprintf(stdout, "Warn: CSocketSrvEpoll recv n < 0 but donnot why \n");
                            myclose(index);
                            // TODO: 通知mainsrv
                            set_MSGID_I2M_CLO_CONNECT(&tmpmsgbuf, index, socketfd);
                            if (pqs->pushmsg(&tmpmsgbuf) <= 0)
                            {
                                fprintf(stderr, "Err: CSocketSrvEpoll close connect msg cannot push into queue !!!!!!\n ");
								return -1;
                            }
                            break;
                        }
                    }
                    else if (n == 0)    // 对端关闭
                    {
                        fprintf(stdout, "Warn: CSocketSrvEpoll recv n = 0 \n");
                        myclose(index);
                        // TODO: 通知mainsrv
                        set_MSGID_I2M_CLO_CONNECT(&tmpmsgbuf, index, socketfd);
                        if (pqs->pushmsg(&tmpmsgbuf) <= 0)
                        {
                            fprintf(stderr, "Err: CSocketSrvEpoll close connect msg cannot push into queue !!!!!!\n ");
							return -1;
                        }
                        break;
                    }

                }  // if (prmsg->n >= 4)
            
            } // while (psi->recvflag == 0)

        }
// ------------------------ 发送数据
        if (events[i].events & EPOLLOUT)
        {
            psi->writable = 1;
        }
    }
    return 0;
}




/* 
int CSocketSrvEpoll::my_epoll_wait_debug_nosend(CShmQueueSingle * pqs, CShmQueueMulti * pqm)
{
    int index;
    int socketfd;
    StMsgBuffer tmpmsgbuf;
	
	// set EPOLLOUT first
	pqm->popmsg_complex(epfd, &(socketlist));
	
	// handle 
    int nfds = epoll_wait(epfd, events, EPOLL_SIZE, EPOLL_TIMEOUT);
    if ( nfds < 0 )
    {
        fprintf(stderr, "Err: CSocketSrvEpoll epoll_wait() \n ");
        fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));// system interupter ?
        return -1;
    }

    for (int i = 0; i < nfds; ++i)
    {
        socketfd = events[i].data.fd;  // 获取fd
        index = socketlist.find(socketfd); // 获取上下文的下标

        if (index < 0)
        {
            fprintf(stderr, "Warn: CSocketSrvEpoll cannot find socketfd=>socketlist.val[] \n ");
            continue;
        }
        CSocketInfo *psi = &(socketlist.val[index]);  // 获取上下文指针
        StMsgBuffer *prmsg = &(psi->recvmsg);
        StMsgBuffer *psmsg = &(psi->sendmsg);
        // int recvflag = psi->recvflag;
        // int sendflag = psi->sendflag;


        //-------------------- 一个新socket用户连接到监听端口，建立新的连接
        if (socketfd == listenfd)
        {
            // fprintf(stdout, "Info: CSocketSrvEpoll new conncet. \n");
            // 建立新连接
            clientAddrLen =  sizeof(struct sockaddr_in);
            int connfd = accept(listenfd, (sockaddr *) &clientaddr, &clientAddrLen );
            if (connfd < 0)
            {
                fprintf(stderr, "Err: CSocketSrvEpoll accept new connect failed. \n");
                fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));
                continue;
            }

            // 设为非阻塞
            if (setnonblocking(connfd) != 0)
            {
                fprintf(stderr, "Err: CSocketSrvEpoll setnonblocking(connfd) \n ");
                fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));
            }

            // 新连接的上下文
            index = socketlist.idle();
            if (index < 0)
            {
                fprintf(stderr, "Err: CSocketSrvEpoll socketlist is full!!! \n");
            }
            socketlist.val[index].srvfd = connfd;

            // 注册新的连接
            ev.data.fd = connfd; // 找到空闲的socketinfo
            ev.events = EPOLLIN | EPOLLET;

            printf("Info: new connect fd = %d, index = %d \n", connfd, index);

            if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) < 0)
            {
                fprintf(stderr, "Err: CSocketSrvEpoll epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) \n ");
                fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));
                continue;
            }

            set_MSGID_I2M_NEW_CONNECT(&tmpmsgbuf, index, connfd);
            if (pqs->pushmsg(&tmpmsgbuf) <= 0)
            {
                fprintf(stderr, "Err: CSocketSrvEpoll new connect msg cannot push into queue !!!!!!\n ");
            }
            continue;
        }

        // ------------------------ 发送数据
        if (events[i].events & EPOLLOUT)
        {
            if (psi->sendflag == 0)  // 没有东西要发送
            {
                fprintf(stderr, "Warn: sendflag == 0 but act EPOLLOUT \n");
                ev.data.fd = socketfd;
                ev.events = EPOLLIN | EPOLLET;
                if (epoll_ctl(epfd, EPOLL_CTL_MOD, socketfd, &ev) < 0)
                    fprintf(stderr, "Err: CSocketSrvEpoll cannot mod fd to EPOLLIN | EPOLLET\n");
            }
            while (psi->sendflag == 1)
            {
                uint32_t msglen = *(uint32_t *)psmsg->buf;
				psmsg->n = msglen; //debug

                    if (psmsg->n == msglen)  // 发送完了，还需要判断通道内是否有数据还要发送
                    {
						if (pqm->popmsg(psmsg) > 0)
						{
							psmsg->n = 0;
							psi->sendflag = 1; // 继续发送
						}
						else // 通道内没数据
						{
							psmsg->n = 0;  // 复位
							psi->sendflag = 0; // 可以接受新的消息
							// 修改为只响应EPOLLIN | EPOLLET
							ev.data.fd = socketfd;
							ev.events = EPOLLIN | EPOLLET;
							if (epoll_ctl(epfd, EPOLL_CTL_MOD, socketfd, &ev) < 0)
								fprintf(stderr, "Err: CSocketSrvEpoll cannot mod fd to EPOLLIN | EPOLLET\n");
						} 
                    }
                
            } // end while ()
        }
        // ------------------- 进行读操作
        else if (events[i].events & EPOLLIN | EPOLLET)
        {
            if (psi->recvflag == 1)  // 已经收取了一个msg，还未取走
            {
                // TODO 压入共享内存区
                CMsgHead *phead = (CMsgHead *)prmsg;
                phead->dstid = socketfd; // read需修改dstid
                if (pqs->pushmsg(prmsg) >= 0)
                {
                    prmsg->n = 0;
                    psi->recvflag = 0;
                }
                else
                {
                    fprintf(stderr, "Err: CSocketSrvEpoll EPOLLIN | EPOLLET but queue is full < 0 !!!!!!\n ");
                }
            }
            while (psi->recvflag == 0)
            {
                if (prmsg->n < 4)  // 还未获取到msglen
                {
                    ssize_t n = recv(socketfd, prmsg->buf + prmsg->n, 4 - prmsg->n, 0); //
                    if (n > 0)
                    {
                        prmsg->n += n;
                    }
                    else if (n < 0)
                    {
                        if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)  // 跳出，等待下次接收
                        {
                            break;// here can just continue
                        }
                        else if (errno == ECONNRESET)     // 对方关闭了socket
                        {
                            fprintf(stdout, "Warn: CSocketSrvEpoll ECONNRESET \n ");
                            myclose(index);
                            // TODO: 通知mainsrv
                            set_MSGID_I2M_CLO_CONNECT(&tmpmsgbuf, index, socketfd);
                            if (pqs->pushmsg(&tmpmsgbuf) <= 0)
                            {
                                fprintf(stderr, "Err: CSocketSrvEpoll close connect msg cannot push into queue !!!!!!\n ");
                            }
                            break;
                        }
                        else
                        {
                            fprintf(stdout, "Warn: CSocketSrvEpoll n < 0 but donot know why \n ");
                            myclose(index);
                            // TODO: 通知mainsrv
                            set_MSGID_I2M_CLO_CONNECT(&tmpmsgbuf, index, socketfd);
                            if (pqs->pushmsg(&tmpmsgbuf) <= 0)
                            {
                                fprintf(stderr, "Err: CSocketSrvEpoll close connect msg cannot push into queue !!!!!!\n ");
                            }
                            break;
                        }
                    }
                    else if (n == 0)    // 对端关闭
                    {
                        fprintf(stdout, "Warn: CSocketSrvEpoll recv n = 0 \n");
                        myclose(index);
                        // TODO: 通知mainsrv
                        set_MSGID_I2M_CLO_CONNECT(&tmpmsgbuf, index, socketfd);
                        if (pqs->pushmsg(&tmpmsgbuf) <= 0)
                        {
                            fprintf(stderr, "Err: CSocketSrvEpoll close connect msg cannot push into queue !!!!!!\n ");
                        }
                        break;
                    }
                }
                if (prmsg->n >= 4)  // 已经获取到msglen
                {
                    uint32_t msglen = *(uint32_t *)prmsg->buf;
                    ssize_t n = recv(socketfd, prmsg->buf + prmsg->n, msglen - prmsg->n, 0); // 保证读取的是一个msg
                    if (n > 0)
                    {
                        prmsg->n += n;
                        if (prmsg->n >= msglen)  // 收取了一个完整msg
                        {
                            CMsgHead *phead = (CMsgHead *)prmsg;
                            phead->dstid = socketfd; // read需修改dstid
                            // TODO 压入共享内存区
                            if (pqs->pushmsg(prmsg) >= 0)
                            {
                                prmsg->n = 0;
                                psi->recvflag = 0;
                            }
                            else
                            {
                                fprintf(stderr, "Err: pushmsg() failed because it's full !!!!\n");
                                // 如果压入共享内存区失败，则先保留在socketinfo的msg缓冲区
                                psi->recvflag = 1; //只是置标志位而已，TODO 这里有问题，应该通过ShmQueue来取走
                            }
                        }
                    }
                    if (n < 0)
                    {
                        if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)  // 跳出，等待下次接收
                        {
                            break;
                        }
                        else if (errno == ECONNRESET)     // 对方关闭了socket
                        {
                            fprintf(stdout, "Warn: CSocketSrvEpoll ECONNRESET \n");
                            myclose(index);
                            // TODO: 通知mainsrv
                            set_MSGID_I2M_CLO_CONNECT(&tmpmsgbuf, index, socketfd);
                            if (pqs->pushmsg(&tmpmsgbuf) <= 0)
                            {
                                fprintf(stderr, "Err: CSocketSrvEpoll close connect msg cannot push into queue !!!!!!\n ");
                            }
                            break;
                        }
                        else
                        {
                            fprintf(stdout, "Warn: CSocketSrvEpoll recv n < 0 but donnot why \n");
                            myclose(index);
                            // TODO: 通知mainsrv
                            set_MSGID_I2M_CLO_CONNECT(&tmpmsgbuf, index, socketfd);
                            if (pqs->pushmsg(&tmpmsgbuf) <= 0)
                            {
                                fprintf(stderr, "Err: CSocketSrvEpoll close connect msg cannot push into queue !!!!!!\n ");
                            }
                            break;
                        }
                    }
                    else if (n == 0)    // 对端关闭
                    {
                        fprintf(stdout, "Warn: CSocketSrvEpoll recv n = 0 \n");
                        myclose(index);
                        // TODO: 通知mainsrv
                        set_MSGID_I2M_CLO_CONNECT(&tmpmsgbuf, index, socketfd);
                        if (pqs->pushmsg(&tmpmsgbuf) <= 0)
                        {
                            fprintf(stderr, "Err: CSocketSrvEpoll close connect msg cannot push into queue !!!!!!\n ");
                        }
                        break;
                    }

                }
            
            } // while (psi->recvflag == 0)

        }
    }
    return 0;
}

 */

/* 
int CSocketSrvEpoll::my_epoll_wait_debug(CShmQueueSingle *pshmQueueSingle)
{
    int index;
    int socketfd;
    StMsgBuffer tmpmsgbuf;

    int nfds = epoll_wait(epfd, events, EPOLL_SIZE, EPOLL_TIMEOUT);
    if ( nfds < 0 )
    {
        fprintf(stderr, "Err: CSocketSrvEpoll epoll_wait() \n ");
        fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));// system interupter ?
        return -1;
    }

    for (int i = 0; i < nfds; ++i)
    {
        socketfd = events[i].data.fd;  // 获取fd
        index = socketlist.find(socketfd); // 获取上下文的下标

        if (index < 0)
        {
            fprintf(stderr, "Warn: CSocketSrvEpoll cannot find socketfd=>socketlist.val[] \n ");
            continue;
        }
        CSocketInfo *psi = &(socketlist.val[index]);  // 获取上下文指针
        StMsgBuffer *prmsg = &(psi->recvmsg);
        StMsgBuffer *psmsg = &(psi->sendmsg);
        // int recvflag = psi->recvflag;
        // int sendflag = psi->sendflag;


        //-------------------- 一个新socket用户连接到监听端口，建立新的连接
        if (socketfd == listenfd)
        {
            // fprintf(stdout, "Info: CSocketSrvEpoll new conncet. \n");
            // 建立新连接
            clientAddrLen =  sizeof(struct sockaddr_in);
            int connfd = accept(listenfd, (sockaddr *) &clientaddr, &clientAddrLen );
            if (connfd < 0)
            {
                fprintf(stderr, "Err: CSocketSrvEpoll accept new connect failed. \n");
                fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));
                continue;
            }

            // 设为非阻塞
            if (setnonblocking(connfd) != 0)
            {
                fprintf(stderr, "Err: CSocketSrvEpoll setnonblocking(connfd) \n ");
                fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));
            }

            // 新连接的上下文
            index = socketlist.idle();
            if (index < 0)
            {
                fprintf(stderr, "Err: CSocketSrvEpoll socketlist is full!!! \n");
            }
            socketlist.val[index].srvfd = connfd;

            // 注册新的连接
            ev.data.fd = connfd; // 找到空闲的socketinfo
            ev.events = EPOLLIN | EPOLLET;

            printf("Info: new connect fd = %d, index = %d \n", connfd, index);

            if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) < 0)
            {
                fprintf(stderr, "Err: CSocketSrvEpoll epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) \n ");
                fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));
                continue;
            }

            set_MSGID_I2M_NEW_CONNECT(&tmpmsgbuf, index, connfd);
            if (pshmQueueSingle->pushmsg(&tmpmsgbuf) <= 0)
            {
                fprintf(stderr, "Err: CSocketSrvEpoll new connect msg cannot push into queue !!!!!!\n ");
            }
            continue;
        }

        // 发送数据
        if (events[i].events & EPOLLOUT)
        {
                uint32_t msglen = *(uint32_t *)psmsg->buf;
                ssize_t n = send(socketfd, psmsg->buf + psmsg->n, msglen - psmsg->n, 0);
                if (n < 0)
                {
                    if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)   // 没有发送完
                    {
                        continue;// 
                    }
                    else     // 出错
                    {
                        fprintf(stderr, "Err: CSocketSrvEpoll send() error happen \n");
						fprintf(stderr, "Err: CSocketSrvEpoll errno = %d (%s) \n ", errno, strerror(errno));
                        myclose(index);
                        continue;
                    }
                }
                else if (n == 0)    // 一般不返回0
                {
                    fprintf(stderr, "Err: CSocketSrvEpoll send() n == 0 \n");
					continue;
                }
                else
                {
                    psmsg->n += n;
                    if (psmsg->n == msglen)  // 发送完了
                    {
                        psmsg->n = 0;  // 复位

                        ev.data.fd = socketfd;
                        ev.events = EPOLLIN | EPOLLET;
                        if (epoll_ctl(epfd, EPOLL_CTL_MOD, socketfd, &ev) < 0)
                            fprintf(stderr, "Err: CSocketSrvEpoll cannot mod fd to EPOLLIN | EPOLLET\n");
                    }
					continue;
                }
            
        }
        // 进行读操作
        else if (events[i].events & EPOLLIN | EPOLLET)
        {
                if (prmsg->n < 4)  // 还未获取到msglen
                {
                    ssize_t n = recv(socketfd, prmsg->buf + prmsg->n, 4 - prmsg->n, 0); //
                    if (n > 0)
                    {
                        prmsg->n += n;
                    }
                    else if (n < 0)
                    {
                        if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)  // 跳出，等待下次接收
                        {
                            continue;// here can just continue
                        }
                        else if (errno == ECONNRESET)     // 对方关闭了socket
                        {
                            fprintf(stdout, "Warn: CSocketSrvEpoll ECONNRESET \n ");
                            myclose(index);
                            continue;
                        }
                        else
                        {
                            fprintf(stdout, "Warn: CSocketSrvEpoll n < 0 but donot know why \n ");
                            myclose(index);
                            continue;
                        }
                    }
                    else if (n == 0)    // 对端关闭
                    {
                        fprintf(stdout, "Warn: CSocketSrvEpoll recv n = 0 \n");
                        myclose(index);
                        continue;
                    }
                }
                if (prmsg->n >= 4)  // 已经获取到msglen
                {
                    uint32_t msglen = *(uint32_t *)prmsg->buf;

                    ssize_t n = recv(socketfd, prmsg->buf + prmsg->n, msglen - prmsg->n, 0); // 保证读取的是一个msg
                    if (n > 0)
                    {
                        prmsg->n += n;
                        if (prmsg->n >= msglen)  // 收取了一个完整msg
                        {
							prmsg->n = 0;
							sta.check("my_epoll_wait_debug");
							


	// CMsgRequestLoginPara *pinpara =
        // (CMsgRequestLoginPara *)(prmsg->buf + sizeof(CMsgHead));	

							
	CMsgResponseLoginPara *poutpara =
        (CMsgResponseLoginPara *)(psmsg->buf + sizeof(CMsgHead));


    poutpara->m_unUin = 1;
    poutpara->m_unSessionID = 2;
    poutpara->m_bResultID = 3;
    strcpy(poutpara->m_stPlayerInfo.m_szUserName, "TestName");

    poutpara->m_stPlayerInfo.m_unUin = 1;
    poutpara->m_stPlayerInfo.m_bySex = 0;
    poutpara->m_stPlayerInfo.m_unLevel = 99;
    poutpara->m_stPlayerInfo.m_unWin = 90;
    poutpara->m_stPlayerInfo.m_unLose = 8;
    poutpara->m_stPlayerInfo.m_unRun = 1;

    CMsgHead * phead = (CMsgHead *)psmsg->buf;
    phead->msglen = sizeof(CMsgHead) + sizeof(CMsgResponseLoginPara);

    phead->msgid = MSGID_REQUESTLOGIN; //16位无符号整型，消息ID
    phead->msgtype = Response;   //16位无符号整型，消息类型，当前主要有Requst、Response以及Notify三种类型
    phead->msgseq = 1234567890;     //32位无符号整型，消息序列号
    phead->srcfe = FE_GAMESVRD ;       //8位无符号整型，消息发送者类型，当前主要有FE_CLIENT、FE_GAMESVRD以及FE_DBSVRD三种
    phead->dstfe = FE_CLIENT;     //8位无符号整型，消息接收者类型 同上
    phead->srcid = socketfd;   //16位无符号整型，当客户端向游戏服务器发送消息时ScrID为SessionID
    phead->dstid = 0;   //16位无符号整型，当游戏服务器向客户端发送消息是DstID为SessionID
							
							
							
							
                            
							ev.data.fd = socketfd;  
							ev.events = EPOLLOUT;
							if (epoll_ctl(epfd, EPOLL_CTL_MOD, ev.data.fd, &ev) < 0)  // 设置发送
							{
								fprintf(stderr, "Err: popmsg_complex epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) \n ");
								fprintf(stderr, "Err: popmsg_complex errno = %d (%s) \n ", errno, strerror(errno));
								return -1;
							}
                        }
                    }
                    if (n < 0)
                    {
                        if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)  // 跳出，等待下次接收
                        {
                            continue;
                        }
                        else if (errno == ECONNRESET)     // 对方关闭了socket
                        {
                            fprintf(stdout, "Warn: CSocketSrvEpoll ECONNRESET \n");
                            myclose(index);
                            continue;
                        }
                        else
                        {
                            fprintf(stdout, "Warn: CSocketSrvEpoll recv n < 0 but donnot why \n");
                            myclose(index);
                            continue;
                        }
                    }
                    else if (n == 0)    // 对端关闭
                    {
                        fprintf(stdout, "Warn: CSocketSrvEpoll recv n = 0 \n");
                        // 关闭socket
                        myclose(index);
                        continue;
                    }

                }
            
            }

        
    }
    return 0;
}

 */
