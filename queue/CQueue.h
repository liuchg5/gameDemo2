#ifndef CQUEUE_H
#define CQUEUE_H
//  不再受最大msg的限制，完全使用可用的字节空间

#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

class CQueue
{
public:
    int NMAX; // 队列的最大长队
    int r, w; // 读写游标  从0 - (NMAX-1)

public:

    void CQueue_Construction(int queue_size)
    {
        NMAX = queue_size;
    }

    void clear()
    {
        w = r = 0;
    }

    int size() // 返回当前可读取字节数
    {
        if (w >= r)
            return (w - r) ;
        else
            return (NMAX - (r - w));
    }

    int full()
    {
        if (size() == NMAX )
            return 1 ;
        else
            return 0;
    }
    int empty()
    {
        if (size() == 0)
            return 1 ;
        else
            return 0;
    }

    

    int push(char *base, char *buf);   // 原子操作，buf不可为NULL

    int pop(char *base, char *buf);  // 原子操作，读取完整msg  遵循的规则是只要非空，就一定有msg可以读取，buf可为空，但plen不能为空
    int pop_just(char *base);
    int top(char *base, char *buf);
    int top_just(char *base);
};
#endif
