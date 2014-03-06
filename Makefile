############################################################
##                                                        ##
##      Makefile for nps				  ##
##                                                        ##
##      Author : SBS		                          ##
##      Date   : 2014.03.05                               ##
##                                                        ##
############################################################

.SUFFIX: .o .cpp .c

CC = g++
CFLAGS = -Wall
LFLAGS = -lpthread


INC =

## Server
SRC_SERVER =	./src/nps.cpp \
		./src/whiteboard/watcher.cpp \
	        ./src/whiteboard/board.cpp \
		./src/util/util.cpp

OBJ_SERVER = $(SRC_SERVER:.cpp=.o)

TARGET_SERVER = ./bin/nps 

		
## Client
TARGET_CLIENT = ./bin/nps_client 

SRC_CLIENT = ./src/client/nps_client.cpp \
	     ./src/whiteboard/watcher.cpp

OBJ_CLIENT = $(SRC_CLIENT:.cpp=.o)

## common

all : server client

clean : 
	rm -rf $(OBJ_CLIENT) ./bin/*
	rm -rf $(OBJ_SERVER)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $(<:.c=.o)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $(<:.cpp=.o)

server : $(OBJ_SERVER)
	$(CC) $(OBJ_SERVER) $(LFLAGS) -o $(TARGET_SERVER) -pthread

client : $(OBJ_CLIENT)
	$(CC) $(OBJ_CLIENT) $(LFLAGS) -o $(TARGET_CLIENT) -pthread


## test code
test :
	$(TARGET_SERVER) &
	$(TARGET_CLIENT)