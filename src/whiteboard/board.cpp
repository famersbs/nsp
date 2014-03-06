#include "board.h"

#include <stdio.h>
#include <unistd.h>
#include <iostream>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <string.h>

#include <sys/wait.h>

NPSWhiteBoard::NPSWhiteBoard( const char* board_name ){
	this->board_name = board_name;
	ctrl_fifo_read_name = this->board_name + "_r";
	ctrl_fifo_write_name = this->board_name + "_w";
}

int NPSWhiteBoard::run(){
	int ret = 0;

	// shm 생성 및 기본 정보 기록

	// 제어 fifo 생성
	unlink(board_name.c_str());

	if ( mkfifo(board_name.c_str(), 0666) < 0) { 
	     printf("\n %s \n", strerror(errno));
	     return 0;
    }

	return ret;
}