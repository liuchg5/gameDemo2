#!/bin/bash
cd common/
make clean; make
cd ..

cd msg
make clean; make
cd ..

cd queue
make clean; make
cd ..

cd socket 
make clean; make
cd ..

cd db_insrv
make clean; make
cd ..

cd db_midsrv
make clean; make
cd ..

cd insrv
make clean; make
cd ..

cd midsrv
make clean; make
cd ..

cd outsrv
make clean; make
cd ..

cd simclient
make clean; make
cd ..

