OBJ_DIR = ./obj
BIN_DIR = ./bin

CC = g++
CXXFLAGS += -std=c++11 -Wall -O3

all: ftp ftp-server
.PHONY: clean

client = client/main.o client/client.o common/util.o
server = server/main.o server/server.o server/handler.o common/util.o

ftp: $(client)
	$(CC) -o $@ $^
ftp-server: $(server)
	$(CC) -o $@ $^

${OBJ_DIR}/%.o: client/%.h
	$(CC) $< -o $@

client/main.o: client/main.cpp
client/client.o: client/client.h
server/main.o: server/main.cpp
server/server.o: server/server.h
server/handler.o: server/handler.o
common/util.o: common/util.h

clean:
	rm client/*.o
	rm server/*.o
	rm common/*o
	rm ftp
	rm ftp-server
