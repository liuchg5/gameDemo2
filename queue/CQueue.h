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
    /*{
        int n, empty_n;
        int len = *((int *)buf);
        n = size();
        empty_n = NMAX - n;
        if (empty_n < len)
            return -1; //剩余的空间不足
        if (buf == NULL)
        {
            return -2;
        }
        //分两种情况压入
        if (w >= r)
        {
            n = (NMAX - w);
            if (n >= len)  // 可以直接写入
            {
                memcpy(base + w, buf, len);
                w += len;
                return len;// 返回写入的长度
            }
            // 要分两次写入
            memcpy(base + w, buf, n);
            memcpy(base, buf + n, len - n);
            w = len - n;
            return len; // 返回写入的长度
        }
        else  // w < r
        {
            // 可以一次写入
            memcpy(base + w, buf, len);
            w += len;
            return len;
        }
    }*/

    int pop(char *base, char *buf);  // 原子操作，读取完整msg  遵循的规则是只要非空，就一定有msg可以读取，buf可为空，但plen不能为空
   /* {
        int n, len;
        n = size();
        if (n == 0)
            return -1;//空队列
        if (n < 4)
        {
            fprintf(stderr, "Err: CQueue's n < 4! \n");
            return -1;
        }
        if (w >= r)
        {
            // 直接读即可
            memcpy(&len, base + r, 4); //获取长度值
            if (len > size()) // 只有长度值
            {
                return -1;
            }
            memcpy(buf, base + r, len);
            r += len;
            return (len);
        }
        else   // w < r
        {
            n = (NMAX - r); //r_to_end();
            // 先获取长度值
            if (n >= 4)
            {
                memcpy(&len, base + r, 4); //获取长度值
            }
            else
            {
                memcpy(&len, base + r, n);
                memcpy(((char *)&len + n), base, 4 - n);
            }
            if (len > size()) // 只有长度值
            {
                return -1;
            }
            if (n >= len)
            {
                memcpy(buf, base + r, len);
                r += len;
            }
            else
            {
                memcpy(buf, base + r, n);
                memcpy(buf + n, base, len - n);
                r = len - n;
            }
            return len;
        }
    }*/
    int pop_just(char *base);
  /*  {
        int n, len;
        n = size();
        if (n == 0)
            return -1;//空队列
        if (n < 4)
        {
            fprintf(stderr, "Err: CQueue's n < 4! \n");
            return -1;
        }
        if (w >= r)
        {
            // 直接读即可
            memcpy(&len, base + r, 4); //获取长度值
            if (len > size()) // 只有长度值
            {
                return -1;
            }
            r += len;
            return (len);
        }
        else   // w < r
        {
            n = (NMAX - r); //r_to_end();
            // 先获取长度值
            if (n >= 4)
            {
                memcpy(&len, base + r, 4); //获取长度值
            }
            else
            {
                memcpy(&len, base + r, n);
                memcpy(((char *)&len + n), base, 4 - n);
            }
            if (len > size()) // 只有长度值
            {
                return -1;
            }
            if (n >= len)
            {
                r += len;
            }
            else
            {
                r = len - n;
            }
            return len;
        }
    }*/
    int top(char *base, char *buf);
   /* {
        int n, len;
        n = size();
        if (n == 0)
            return -1;//空队列
        if (n < 4)
        {
            fprintf(stderr, "Err: CQueue's n < 4! \n");
            return -1;
        }
        if (w >= r)
        {
            // 直接读即可
            memcpy(&len, base + r, 4); //获取长度值
            if (len > size()) // 只有长度值
            {
                return -1;
            }
            memcpy(buf, base + r, len);
            //r += len;
            return (len);
        }
        else   // w < r
        {
            n = (NMAX - r); //r_to_end();
            // 先获取长度值
            if (n >= 4)
            {
                memcpy(&len, base + r, 4); //获取长度值
            }
            else
            {
                memcpy(&len, base + r, n);
                memcpy(((char *)&len + n), base, 4 - n);
            }
            if (len > size()) // 只有长度值
            {
                return -1;
            }
            if (n >= len)
            {
                memcpy(buf, base + r, len);
                //r += len;
            }
            else
            {
                memcpy(buf, base + r, n);
                memcpy(buf + n, base, len - n);
                // r = len -n;
            }
            return len;
        }
    }*/
    int top_just(char *base);
/*    {
        int n;
        int len = 0;
        n = size();

        if (n == 0)
            return -1;//空队列

        if (n < 4)
        {
            // fprintf(stderr, "Err: CQueue's n < 4! \n");
            return -1;
        }
        if (w >= r)
        {
            // 直接读即可
            memcpy(&len, base + r, 4); //获取长度值
            if (len > size()) // 只有长度值
            {
                return -1;
            }
            // memcpy(buf, base+r, len);
            //r += len;
            return (len);
        }
        else   // w < r
        {
            n = (NMAX - r); //r_to_end();
            // 先获取长度值
            if (n >= 4)
            {
                memcpy(&len, base + r, 4); //获取长度值
            }
            else
            {
                memcpy(&len, base + r, n);
                memcpy(((char *)&len + n), base, 4 - n);
            }
            if (len > size()) // 只有长度值
            {
                return -1;
            }
            // if (n >= len){
            //           // memcpy(buf, base+r, len);
            //           //r += len;
            //    }else{
            //           memcpy(buf, base+r, n);
            //           memcpy(buf+n, base, len - n);
            //          // r = len -n;
            //    }
            return len;
        }
    }*/
};
#endif
