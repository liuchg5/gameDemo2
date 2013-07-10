
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