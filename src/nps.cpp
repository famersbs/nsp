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

using namespace std;

#define myFIFO "/tmp/nps_fifo"

int readFifo(){

	int num, fifo = -1, status;
	int temp_int = 0;

    if ( ( fifo = open(myFIFO, O_RDONLY) ) < 0) { 
	     printf("\n %s \n", strerror(errno));
	     return 0;
    }

	for( int i = 0 ; i < 100 ; ++ i ){
		if ( ( num = read(fifo, &temp_int, sizeof(int)) ) < 0) { 
			 printf("\n %s \n", strerror(errno));
			 return 0;
		}

		printf("In FIFO is %d \n", temp_int);
	}

	printf("In FIFO end \n");
}

int writeFifo(){

    int status, num, fifo = -1;
    //char string[]="Testing...";


	 if ( ( fifo = open(myFIFO, O_RDWR) ) < 0) { 
		 printf("\n %s \n", strerror(errno));
		 return 0;
	 }

	for( int i = 0 ; i < 100 ; ++ i ){

		 if ( ( num = write(fifo, &i, sizeof(int) ) ) < 0) { 
			 printf("\n %s \n", strerror(errno));
			 return 0;
		 }
	 	cout << "write Fifo " << i << " "  << fifo <<endl;
		sleep(1);
	}

	close( fifo );

	cout << "write Fifo end..."<<endl;

}

int main( int argc, char* argv[] ){

	unlink(myFIFO);

	if ( mkfifo(myFIFO, 0666) < 0) { 
	     printf("\n %s \n", strerror(errno));
	     return 0;
    }

	// read 로직
	int pid_read   = fork();

	if( 0 == pid_read ){
		readFifo();
		return 0;
	}

	// write 로직
	int pid_write = fork();
	if( 0 == pid_write ){
		writeFifo();
		return 0;
	}

	int status = 0;
	int options = 0;


	// wating
	waitpid( pid_write, &status, options );
	waitpid( pid_read, &status, options );

	cout << "End Process" << endl;

}
