#ifndef CMAP_H
#define CMAP_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

class CMap //
{
public:
    int size;
    int *pindex;
    int *psrvfd;

    CMap(int epoll_size);
    ~CMap();

    void clear();

    int reg(int index, int srvfd);
    int fnd(int *pin, int srvfd);
    int del(int index);
};




#endif
