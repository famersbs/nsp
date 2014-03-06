#ifndef ________BOARD____________
#define ________BOARD____________

#include <string>

class NPSWhiteBoard
{
	private:
		std::string board_name;

		std::string ctrl_fifo_read_name;
		std::string ctrl_fifo_write_name;

	public:
		NPSWhiteBoard( const char* board_name );

	public:
		int run();
};


#endif