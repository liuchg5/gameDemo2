CPDIR = ../debug/ ../insrv/  ../midsrv/ ../outsrv/ ../simclient  ../client  ../db_insrv/ ../db_midsrv/
TARGET = all


CC = gcc
XX = g++
CFLAGS = -Wall -O -pg -g



SOURCES = $(wildcard *.c *.cpp)	
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))

$(TARGET): $(OBJS)
	for i in ${CPDIR}; do	\
	cp *.o $$i;	\
	done
	
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(XX) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o 
	for i in $(CPDIR); do	\
		cd $$i ;		\
		for j in $(OBJS); do \
			rm -rf $$j ;	\
		done; \
	done



