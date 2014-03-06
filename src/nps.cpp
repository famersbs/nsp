#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <string.h>

#include <sys/wait.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include "util/util.h"


using namespace std;

#define myFIFO "/tmp/nps_fifo"

int readFifo(){

	int fifo = -1, num;
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

	return 0;
}

int writeFifo(){

    int fifo = -1, num;
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
		//sleep(1);
	}

	close( fifo );

	cout << "write Fifo end..."<<endl;

	return 0;
}

// fifo test
int fifo_test(){

	waitProcessMan man;

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
	
	man.addProce( pid_read );


	// write 로직
	int pid_write = fork();
	if( 0 == pid_write ){
		writeFifo();
		return 0;
	}

	man.addProce( pid_write );

	man.wait();

	return 0;
}

// msgq test
#define MSGQ_READTHREAD_CNT	10
#define MSGQ_KEY			1234
typedef struct {
   long  data_type;
   int   data_num;
} t_data;

int msgq_test(){

	// read_mq 
	int read_prcid[MSGQ_READTHREAD_CNT] = {0,};
	for( int i = 0 ; i < MSGQ_READTHREAD_CNT ; ++ i ){
		int pid = fork();
		if( 0 == pid ){

			pid_t pid = getpid();

			cout << "reader " << pid << " start"<<endl;

			int      msqid;
			t_data   data;

			if ( -1 == ( msqid = msgget( (key_t)MSGQ_KEY, IPC_CREAT | 0666)))
			{
			  perror( "msgget() 실패");
			  exit( 1);
			}

			while( 1 )
			{
			  // 메시지 큐 중에 data_type 이 2 인 자료만 수신
			  if ( -1 == msgrcv( msqid, &data, sizeof( t_data ) - sizeof( long), -1, 0))
			  {
				 perror( "msgrcv() 실패");
				 exit( 1);
			  }

			  printf( "reader[%u] %d\n", (int)pid , data.data_num);
  			  if( data.data_num == -1 ) break;
			}
			cout << "reader " << pid << " close"<<endl;
			exit(0);
			return 0;
		}else{
			read_prcid[i] = pid;
		}
	}

	// write_mq
	int write_pid = fork();

	if( 0 == write_pid ){
		
		sleep(1);	// reader 가 다 생성되길 기다립니다.
		cout << "Start Writer" << endl;
		int      msqid;
		t_data   data;

		if ( -1 == ( msqid = msgget( (key_t)MSGQ_KEY, IPC_CREAT | 0666)))
		{
		  perror( "msgget() 실패");
		  exit( 1);
		}

		for( int i = 0 ; i < 1 ; ++ i ){

		  cout << "Writer "<< i << endl;

		  data.data_type = 1;
		  data.data_num  = i;

		  if ( -1 == msgsnd( msqid, &data, sizeof(t_data) - sizeof( long ), 0))
		  {
			 perror( "msgsnd() 실패");
			 exit( 1);
		  }
		}

		data.data_type = 1;
		data.data_num  = -1;
		if ( -1 == msgsnd( msqid, &data, sizeof(t_data) - sizeof( long ), 0))
		{
		 perror( "msgsnd() 실패");
		 exit( 1);
		}

		cout << "End Writer" << endl;

		exit(0);
		return 0;
	}

	int status = 0;
	int options = 0;
	waitpid( write_pid, &status, options );
	for( int i = 0 ; i < MSGQ_READTHREAD_CNT; ++ i ){
		waitpid( read_prcid[i], &status, options );
	}


	return 0;
}

// Maker, watcher Test
#define CTRL_CODE_ADD_WATCHER		10
#include <string>
#include <vector>

typedef struct 
{
	pid_t	pid;
} req_e;
typedef struct 
{
	int	result;
} job_e;
typedef union {
	req_e r;
	job_e j;
} sub_data;

typedef struct {
	long		watcher_id;
	sub_data	sub;
} t_board_q;

typedef struct{
	pid_t	pid;
	key_t	use_key;
	int		status;		// 0 만들어짐, 1 사용됨
	
} CONTROL_BLOCK;

void printblock( CONTROL_BLOCK* block ){
	cout << "block " << (void*) block << " " << block->pid << " " << block->use_key << " " << block->status << endl;
}

class Marker 
	: public npsRunable
{
	private:
		key_t control_mq_key;
		key_t work_mq_key;
		key_t ctrl_shm_key;
//		int  msqid;

	private:
		npsSHM_CountBlock shm_block;

	private:
		// 이벤트 받을 사람이 늘어 나면 인덱스를 증가 시켜서, 0 부터 cur_watcher_id_seq - 1 까지 메시지를 전송 한다.
		int cur_watcher_id_seq;	

	public:
		Marker( key_t control_mq_key, key_t work_mq_key, key_t ctrl_shm_key, size_t block_size );

	public:
		void run_marker();
		virtual int run();

	private:
		void sendAll(int msqid);
};
Marker::Marker(  key_t control_mq_key, key_t work_mq_key, key_t ctrl_shm_key, size_t /*block_size*/ )
: cur_watcher_id_seq( 1 )
{
	this->control_mq_key = control_mq_key;
	this->work_mq_key = work_mq_key;
	this->ctrl_shm_key = ctrl_shm_key;
};

// 제어 신호 받기
int Marker::run(){

	t_board_q	board_q;
	npsMQ< t_board_q > control_mq;

	cout<< "start Marker Control MQ..." << endl;

	// MSGQ 생성
	if ( -1 == control_mq.open( control_mq_key ) )
	{
		perror( "msgget() 실패");
		exit( 1);
	}
	
	// 제어 매시지 받기
	while(1){

		if ( -1 == control_mq.recv( &board_q ) )
		{
			perror( "msgrcv() 실패");
			exit(1);
		}

		cout<< "rcv control mq : " << board_q.sub.r.pid << endl;

		// PID 를 받았으면 제어키를 만들어야잊~!!
		key_t cur = cur_watcher_id_seq ++;

		int		now_found_key = 1;
		while( now_found_key ) {

			// 쉐어드 매모리 빈영역에 현재 키와 pid를 기록 한다.
			for( int i = 0 ; i < shm_block.length() ; ++ i )
			{
				void* block = shm_block.getBlock(i);
				if( NULL == block ){ perror("shm block is NULL " ); 	exit(1); }

				CONTROL_BLOCK* control = (CONTROL_BLOCK*) block;

				if( 0 == control->status ){
					control->use_key = cur;
					control->pid	 = board_q.sub.r.pid;
					control->status	 = 1;	// 상태 변환 ( 사용 중 )
					now_found_key = 0;
					break;
				}
			}
		}

	}

	return 0;
};



void Marker::run_marker(){

	// 신호 블럭 생성
	shm_block.create( ctrl_shm_key, sizeof(CONTROL_BLOCK), 100  );
	cout<< "control shm Created length " << shm_block.length() << endl;
	
	//t_board_q	board_q;

	// ControlThread 생성
 	( new npsThread(1) )->start( this );

	// MSGQ 생성
	npsMQ< t_board_q > work_mq;

	if ( -1 == work_mq.open( work_mq_key ) )
	{
		perror( "msgget() 실패");
		exit( 1);
	}

	cout<< "marker Start " << work_mq_key <<  endl;

	// 일단 Wating
	while(1){

		t_board_q	board_q;

		for( int i = 0 ; i < cur_watcher_id_seq -1 ; ++ i ){
			board_q.watcher_id = i + 1;
			if ( -1 == work_mq.send( &board_q ) )
			{
				perror( "msgsnd() 실패");
				exit(1);
			}
			cout<< "send " << i << endl;
		}
		sleep(1);

	}
};

void Marker::sendAll(int msqid){
	

}

class Watcher
{
	private:
		key_t control_mq_key;
		key_t work_mq_key;
		int  msqid;
		key_t watcher_key;
		key_t ctrl_shm_key;

	public:
		Watcher( key_t control_mq_key, key_t work_mq_key, key_t ctrl_shm_key, int cnt);

	public:
		void run();

	public:
		int		work();

	private:
		int	watcherKey();
};

// Key 받아오기
int Watcher::watcherKey(){

	pid_t pid = getpid();
	
	// shm 접속
	npsSHM_CountBlock shm_block;
	shm_block.connect(ctrl_shm_key, sizeof(CONTROL_BLOCK), 100  );

	npsMQ< t_board_q > control_mq;

	// MSGQ 생성
	if ( -1 == control_mq.open( control_mq_key ) )
	{
		perror( "msgget() 실패");
		exit( 1);
	}

	// event ID 요청
	t_board_q	tmp;
	tmp.watcher_id = CTRL_CODE_ADD_WATCHER;
	tmp.sub.r.pid = pid;

	cout<< "watcher Start " << msqid <<  endl;

	if ( -1 == control_mq.send( &tmp ) )
	{
		perror( "Watcher - msgsnd() 실패");
		exit(1);
	}

	// sharedMemory 검색
	int is_active = 1;
	while( is_active == 1 ){
		for( int i = 0 ; i < shm_block.length() ; ++ i )
		{
			void* block = shm_block.getBlock( i );
			cout<< block << endl;
			if( NULL == block ){
				perror("shm block is NULL " );
				exit(1);
			}

			CONTROL_BLOCK* control = (CONTROL_BLOCK*) block;

			if( pid == control->pid ){
				watcher_key = control->use_key;
				control->status = 0;	// 사용 된것으로 표시 
				is_active = 0;
				break;
			}
		}

		if( is_active == 1 ) sleep(1);
		cout<< "wait shared memory : " << pid << endl;
	}
	cout<< "Watcher Key Accept : " << pid << " " << watcher_key << endl;

	return 1;
}

Watcher::Watcher( key_t control_mq_key, key_t work_mq_key, key_t ctrl_shm_key, int cnt){
	this->control_mq_key	= control_mq_key;
	this->work_mq_key		= work_mq_key;
	this->ctrl_shm_key		= ctrl_shm_key;

	msqid = 0;
	this->watcher_key = cnt + 1;
}

// 신호 기다려서 일 시작 하기 ~!
void Watcher::run(){

	// event id 등록
	if( 1 != watcherKey() ){
		return;
	}

	// 일단 등록 요청을 한다.
	t_board_q	board_q;
	npsMQ< t_board_q > work_mq;

	// MSGQ 생성
	if ( -1 == work_mq.open( work_mq_key ) )
	{
		perror( "msgget() 실패");
		exit( 1);
	}

	cout<< "watcher rcv " << watcher_key << endl;

	while(1){
		// 해당 watcher key 를 이용하여 메시지 대기
		if ( -1 == work_mq.recv( &board_q, watcher_key ) )
		{
			perror( "msgrcv() 실패");
			exit(1);
		}
		// 이벤트를 받으면 ~!
		work();
	}

};

int Watcher::work(){
	cout<< "work " << watcher_key << endl;

	return 0;
};


#define MW_CONTROL_MQ		1234
#define MW_WORK_MQ			1235
#define MW_CONTROL_SHM		1234

int marker_watcher_test(){
	waitProcessMan man;

	// marker 생성
	int pid = fork();
	if( 0 == pid ){
		Marker m( (key_t)MW_CONTROL_MQ, (key_t) MW_WORK_MQ, (key_t)MW_CONTROL_SHM, 10 );
		m.run_marker();
		exit(1);
	}
	man.addProce( pid );

	sleep(1);

	// watcher 생성
	for( size_t i = 0 ; i < 10; ++ i ){
		pid = fork();
		if( 0 == pid ){
			Watcher w( (key_t)MW_CONTROL_MQ, (key_t) MW_WORK_MQ, MW_CONTROL_SHM, i );
			w.run();
			exit(1);
		}
		man.addProce( pid );
	}

	// 대기
	man.wait();

	return 0;
}


////////////////////////////////////////
// 매인
int main( int argc, char* argv[] ){

	
	//fifo_test();
	//msgq_test();
	marker_watcher_test();

	cout << "End Process" << endl;

}
