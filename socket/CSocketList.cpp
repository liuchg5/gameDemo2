
#include "CSocketList.h"



CSocketList::CSocketList(int epoll_size):
    EPOLL_SIZE(epoll_size)
{
    val = new CSocketInfo[EPOLL_SIZE + 1];
    if (val == NULL)
    {
        fprintf(stderr, "Err: CSocketList new CSocketInfo[EPOLL_SIZE + 1] \n ");
        fprintf(stderr, "Err: CSocketList errno = %d (%s) \n ", errno, strerror(errno));
    }
}

CSocketList::~CSocketList()
{
    delete [] val;
    val = NULL;
}

