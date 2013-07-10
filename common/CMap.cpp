
#include "CMap.h"


CMap::CMap(int epoll_size)
{
    size = epoll_size + 1;
    pindex = new int[size];
    psrvfd = new int[size];
    if (pindex == NULL || psrvfd == NULL)
    {
        fprintf(stderr, "Err: CMap new failed \n" );
        return;
    }
    memset(pindex, 0, sizeof(int) * size);
    memset(psrvfd, 0, sizeof(int) * size);
}


CMap::~CMap()
{
    delete [] pindex;
    delete [] psrvfd;
}


void CMap::clear()
{
    memset(pindex, 0, sizeof(int) * size);
    memset(psrvfd, 0, sizeof(int) * size);
}




int CMap::reg(int index, int srvfd)
{
    for (int i = 1; i < size; ++i)
    {
        if (pindex[i] == index)
        {
            return -1;
        }
    }
    for (int i = 1; i < size; ++i)
    {
        if (pindex[i] == 0)
        {
            pindex[i] = index;
            psrvfd[i] = srvfd;
            return i;
        }
    }
    return -1;
}
int CMap::fnd(int *pin, int srvfd)
{
    for (int i = 1; i < size; ++i)
    {
        if (psrvfd[i] == srvfd)
        {
            *pin = pindex[i];
            return i;
        }
    }
    *pin = -1;
    return -1;
}
int CMap::del(int index)
{
    for (int i = 1; i < size; ++i)
    {
        if (pindex[i] == index)
        {
            pindex[i] = 0;
            psrvfd[i] = 0;
            return i;
        }
    }
    return -1;
}

