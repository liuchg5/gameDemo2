#ifndef CSTATISTICS_H
#define CSTATISTICS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

class CStatistics
{
public:
    long l_interval; //us
    long l_count;

    struct timeval st_start;
    struct timeval st_end;

    int flag;
    int status;//0-init, 1-check

    CStatistics();
    ~CStatistics();

    void set_interval(long val_us)
    {
        l_interval = val_us;
    }
    long get_interval()
    {
        return l_interval;
    }
    void enable()
    {
        flag = 1;
        status = 0;
    }
    void disable()
    {
        flag = 0;
    }
    int check(long *pval, long *ptimeuse);   // 返回0表示到了时间间隔，其他表示未到时间间隔
    void check();
    void check( const char *str);



};





#endif
