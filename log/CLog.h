#ifndef CLOG_H
#define CLOG_H

//单例
class CLog
{
public:
	void set(int level, FILE * set, FILE * val);
	

private:
	

};

extern CLog log; // 作为全局变量

#endif
