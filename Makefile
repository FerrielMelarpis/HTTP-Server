CC=g++
CFLAGS=-std=c++11 -Wall -g -O2
OBJ=httpserver.o server.o
server : $(OBJ)
	$(CC) $(CFLAGS) -o server $(OBJ)
$(OBJ) : httpserver.h

.PHONY:clean
clean:
	rm -rf httpserver.o server.o server