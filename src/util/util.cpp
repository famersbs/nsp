#include "util.h"

#include <sys/wait.h>

//std::vector<int> lists;

void waitProcessMan::addProce( int pid ){
	lists.push_back( pid );
}
void waitProcessMan::wait(){

	std::vector<int>::const_iterator  i = lists.begin();

	int status = 0;
	int options = 0;

	while( lists.end() != i ){

		waitpid( (*i), &status, options );
		
		++ i;
	}

}

//--------------------------------------------------------------------
// Thread
//npsRunable* runer;
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

npsThread::npsThread( int auto_clean ){
	this->auto_clean = auto_clean;
}

int npsThread::start( npsRunable* runner ){

	int 			ret;

	this->runner = runner;

	/*
	memset(&tattr, 0, sizeof(pthread_attr_t));

	ret = pthread_attr_init(&tattr);
	if ( ret != 0 ) {
		printf("startGatewayThread : pthread_attr_init errno=[%d] [%s]", errno, strerror(errno));
		return -1;
	}
	

	ret = pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
	if ( ret != 0 ) {
		printf("startGatewayThread : pthread_attr_setdetachstate errno=[%d] [%s]", errno, strerror(errno));
		return -1;
	}
	*/

	ret = pthread_create(&tid, NULL, &npsThread::running, this);
	if ( ret != 0 ) {
		printf("startGatewayThread : pthread_create errno=[%d] [%s]", errno, strerror(errno));
		return -1;
	}

	printf("tid : %d\n", ret );

	return 0;
}


void *npsThread::running(void* self_){

	npsThread* self = (npsThread*) self_;

	// 실행
	self->runner->run();
	
	// 종료
	if( 1 == self->auto_clean ){
		delete self;
	}

	return NULL;
}


//--------------------------------------------------------------------
// SHM Block Counter
/*
	// data 영역
	private:
		size_t	*count;				// header
		size_t	*block_size;		// header
		void 	*shared_memory;		// body 



	private:
		key_t	shm_key
		int 	shmid;
*/

/** 종료시 모드에 따라 SHM을 삭제 한다 **/
npsSHM_CountBlock::~npsSHM_CountBlock(){
	if( 0 == mode ){
		printf( "shm delete !!! - not now\n");
	}
}

size_t	npsSHM_CountBlock::getTotalBlockSize(){
	return ( _block_size * _block_cnt ) + ( sizeof(size_t) * 2 );
}

int npsSHM_CountBlock::openshm( key_t shm_key, size_t block_size, size_t block_cnt, int shmflg ){

	this->shm_key = shm_key;
	_block_size = block_size;
	_block_cnt = block_cnt;
		
	// SHM 영역 open
	shmid = shmget( (key_t)shm_key, getTotalBlockSize(), shmflg );
	if ( shmid == -1 ) {
		return -1;
	}

	shared_memory = shmat(shmid, (void *)0, 0);
	if ( shared_memory == (void *)-1 ) {
		return -1;
	}

	// 영역 초기화
	this->count			= (size_t*) shared_memory;
	this->block_size	= (size_t*) this->count + 1;
	shared_memory		= (void*)	( this->block_size + 1 );

	return 0;
}

int npsSHM_CountBlock::create( key_t shm_key, size_t block_size, size_t block_cnt, int shmflg ){

	mode = 0;

	// 열기
	if( -1 == openshm( shm_key, block_size, block_cnt, shmflg ) ){
		return -1;
	}

	// 값 쓰기
	*(this->count)		= block_cnt;
	*(this->block_size) = block_size;
	memset( shared_memory, 0x00, getTotalBlockSize() - ( sizeof(size_t) * 2 ) );

	return 0;
}

int npsSHM_CountBlock::connect(  key_t shm_key, size_t block_size, size_t block_cnt, int shmflg ){

	mode = 1;

	// 열기
	if( -1 == openshm( shm_key, block_size, block_cnt, shmflg ) ){
		return -1;
	}

	return 0;
}

int npsSHM_CountBlock::length(){
	return (int)(*count);
}
void* npsSHM_CountBlock::getBlock( size_t index ){

	// 범위 검사
	if( index >= (size_t)length() || index < 0 ){
		return NULL;	// error
	}

	return (void*) ( ((char*)shared_memory ) + ( index * (*block_size) ) );
}