#ifndef ________UTIL____________
#define ________UTIL____________

#include <vector>

class waitProcessMan
{
	private:
		std::vector<int> lists;

	public:
		void addProce( int pid );
		void wait();
};



//--------------------------------------------------------------
// Thread Util
#include <pthread.h>

/** 실행 대상이 상속 받을 인터페이스 **/
class npsRunable
{
	public:
		virtual int run() = 0;
};

/** 
	Thread 객체 

	: 지속적으로 Thread를 관리 할 수 있도록 하는 객체들을 보관 및 관리
**/
class npsThread
{

	private:
		npsRunable* runner;
		int			auto_clean;

	private:
		pthread_attr_t 	tattr;
		pthread_t 		tid;

	public:
		/**
		 *
		 * @param auto_clean  1 이면 Thread가 종료될 경우 delete 된다.
		 **/
		npsThread( int auto_clean = 1 );

	public:
		/**
		 *
		 * @param runner	실행 대상 객체
		 */
		int start( npsRunable* runner );


	public:
		static void *running( void *self );
};

//--------------------------------------------------------------
// MQ for structer
#include <sys/types.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
template <class T>
class npsMQ
{
	private:
		int msqid;

	public:
		int open( key_t key , int flag = IPC_CREAT | 0666 ){
				// MSGQ 생성
			msqid = msgget( key, flag );

			if ( -1 == msqid )
			{
				return -1;
			}

			return 0;
		}

	public:
		int send( T* msg ){
			if ( -1 == msgsnd( msqid, msg, sizeof( T ) - sizeof( long ), 0))
			{
				return -1;
			}
			return 0;
		};
		int recv( T* buf, long type = 0 ){
			if ( -1 == msgrcv( msqid, buf, sizeof( T ) - sizeof( long ), type, 0))
			{
				return -1;
			}
			return 0;
		};
};

//--------------------------------------------------------------
// SHM for structer
#include <sys/shm.h>

class npsSHM_CountBlock
{
	// mode
	private:
		int		mode;				// 0 create. 1 read 

	// data 영역
	private:
		size_t	*count;				// header
		size_t	*block_size;		// header
		void 	*shared_memory;		// body 

	private:
		key_t	shm_key;
		int 	shmid;

	private:
		size_t	_block_size;
		size_t	_block_cnt;
		size_t	getTotalBlockSize();
		int		openshm( key_t shm_key, size_t block_size, size_t block_cnt, int shmflg );

	public:
		~npsSHM_CountBlock();

	public:
		// 블럭을 생성하고 해더 영역을 셋팅
		int		create( key_t shm_key, size_t block_size, size_t block_cnt, int shmflg = IPC_CREAT | 0666 );

		// 블럭에 접속 하고 해더 영역에서 정보를 추출
		int		connect(  key_t shm_key, size_t block_size, size_t block_cnt, int shmflg = IPC_CREAT | 0666 );

	public:
		int		length();
		// getBlock으로 리턴된 메모리는 실제 shm 이므로 조작이 가능하다.
		void*	getBlock( size_t index );	

};

#endif