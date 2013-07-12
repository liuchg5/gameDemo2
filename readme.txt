
2013-07-10
finish epoll framework
4000 connections, 9000 msg per second for each recv and send
cpu id is almost 0, si is about 10, mean the network is 10% use
at present, it just has insrv and midsrv 2 layers.

2013-07-10 Daytime
add "CSocketCli.cpp .h "
add "outsrv.cpp"
mod "midsrv.cpp" and "midhandle.cpp"
add "db*" folder and mod related file
add "db_insrv"
add "db_midsrv"
finish gmsrv(insrv, midsrv, outsrv) and dbsrv(insrv, midsrv)
every srv can run but nothing print out!!!

2013-07-10 Night 
still cannot find why the whole process is very slow!!!
if outsrv donot send msg by socket, 
instead of that, if outsrv just handle msg without socket to dbsrv, then it fast!!!
maybe the problem is "CSocketCli"!!!!!

2013-07-11 Day
find one problem: it is epoll_wait(), timeout should be 0 to immediate return, it just solve one connect slow problem.
prepare do expriment for each layer return msg, now have the outsrv, midsrv two layers.
simclient in the same vm:  (meanless: at the same machine)
	about 200,000 msg per 5s (insrv) 40 connect
	about 110,000 msg per 5s (insrv midsrv)  30 connect
	about 5,000 msg per 5s (insrv midsrv outsrv) 3 connect
Client(java) in host machine:
	about 39499 msg per 5s (insrv) 50 connect id 30
	about 36296 msg per 5s (insrv midsrv) 50 connect id 20
	about 12037 msg per 5s (insrv midsrv outsrv) 50 connect (10 bottleneck) id 50
mod "CMsgHead" "CMsgPara" #pragma pack(1)//设定为1字节对齐 
add java client

==== 2013-07-11 Night ====
prepare test Single Queue, mod midsrv and outsrv
result: Single Queue here just 5000 msg per 5s (10000 for dual queue) 
Because handle one msg from queue then sleep() so it is so slow...
if handle all msg in queue then will faster!!! 
update:
Client(java) in host machine:
	about 39499 msg per 5s (insrv) 50 connect id 30
	about 36296 msg per 5s (insrv midsrv) 50 connect id 20
	about 41330 msg per 5s (insrv midsrv outsrv) 50 connect (40 bottleneck) id 20
but using dbsrv it will be 5000 msg per 5s 
because one connect EPOLL just can handle!!!

===== 2013-07-12 Day =====
find the problem: multi queue just put one msg into srv then srv just send one msg one time then sleep.
check the ideaing...
lookup the shm max: $cat /proc/sys/kernel/shmmax 
set the shm max: $echo "134217728" > /proc/sys/kernel/shmmax  #(128MB)
ipcs -m; ipcrm -m shmid;
Client(java) in host machine:
	about 7500 msg per 1s (insrv midsrv outsrv) 
	about 1700 msg per 1s (insrv midsrv outsrv db_insrv db_midsrv)
if not set EPOLLOUT(just discard the msg), it will be very fast!!! 
deal from this point.