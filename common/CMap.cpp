
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
    // printf("reg(): index = %d, srvfd = %d\n", index, srvfd);
    for (int i = 1; i < size; ++i)
    {
        if (pindex[i] == index)
        {
            fprintf(stderr, "Err: CMap reg() index is exist!!!\n");
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
    fprintf(stderr, "Err: CMap reg() is full !!!\n");
    return -1;
}
int CMap::fnd(int *pin, int srvfd)
{
    if (srvfd == 0)
    {
        fprintf(stderr, "Err: CMap fnd() srvfd == 0 !!!\n");
        *pin = -1;
        return -1;
    }
    for (int i = 1; i < size; ++i)
    {
        if (psrvfd[i] == srvfd)
        {
            *pin = pindex[i];
            return i;
        }
    }
    fprintf(stderr, "Err: CMap fnd() cannot find !!!\n");
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
    fprintf(stderr, "Err: CMap del() index cannot find !!!\n");
    return -1;
}

