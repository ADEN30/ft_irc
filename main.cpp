#include <iostream>
#include <netinet/in.h> /* sockaddr_in*/
#include <sys/socket.h> /* Tout ce qui est lie aux socket*/
#include <vector>
#include "Server.hpp"

int main(int argc, char** argv)
 {
	try
	{
		if (argc != 3)
			throw(std::string("Verify you exec like this: ./ircserv <port> <password>"));
		Server _s(argv[2], argv[1]); 
		_s.run_serv();
	}
	catch(std::string e)
	{
		std::cerr << e << '\n';
	}


}