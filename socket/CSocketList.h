#ifndef CSOCKETLIST_H
#define CSOCKETLIST_H

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "CSocketInfo.h"

class CSocketList // 上下文环境
{
public:
    int EPOLL_SIZE;
    CSocketInfo *val; //[EPOLL_SIZE + 1]; //第一个用来做监听，即下标为0
public:
    CSocketList(int epoll_size);
    ~CSocketList();

    int idle()  // 返回可用的下标，0表示满
    {
        for (int i = 1; i < (EPOLL_SIZE + 1); ++i)
        {
            if (val[i].srvfd < 0)
                return i;
        }
        return -1;
    }
    int find(int fd) // 找到fd对应的下标
    {
        for (int i = 0; i < (EPOLL_SIZE + 1); ++i)
        {
            if (val[i].srvfd == fd)
                return i;
        }
        return -1;
    }
    void erase(int index) // 擦除下标index的元素
    {
        val[index].clear();
    }
    void clear()
    {
        for (int i = 0; i < (EPOLL_SIZE + 1); ++i)
        {
            val[i].clear();
        }
    }
};





#endif
