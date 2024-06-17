#ifndef SERVER_HPP
#define SERVER_HPP
#include "User.hpp"
#include <iostream>
#include <vector>
#include <sys/types.h> /* Utilise certaines macros */
#include <unistd.h> /* */
#include <errno.h>
#include <netdb.h> /* gethostbyname*/
#include <arpa/inet.h>
#include <utility>
#define _BUFF_SIZE 513

class User;
class Chan;

typedef  std::map<int, User>::iterator _ituser;
class Server
{
	private:
		addrinfo *_host;
		char _hostname[256];
		int _sock_serv;
		std::map<int, User> _users;
		std::vector<Chan*> _chan;
		pollfd* _poll;
		size_t _size_poll;
		std::vector<std::string> _rpl;
		std::vector<int> _sendfd;
		std::string _bufferread;
		std::string _cmd;
		std::vector<std::string> _cmdparse;
		int _findtwopoint;
		std::string _pass;
		std::string _port;

	public:
		//constructor
		Server(std::string, std::string);
		~Server();

		//get something
		addrinfo& getaddrinfo_serv();
		int getsock_serv();
		int getRevents();
		std::string gethostname_srv();
		std::map<int, User>& getUsers();

		//find somthings
		User* findUser(int);
		User* findUserbyname(std::string);

		//delete something
		void deleteChan(Chan*);

		//Check something
		Chan* already_channel(std::string);
		void check_port();

		//make/set/run/build something
		void set_channel(Chan*);
		void set_host();
		void makepollfd_fds();
		void make_sockserv();

		//Read/Write/Connection socket
		void run_serv();
		void readfds_serv(int);
		void sendfds_serv();
		void accept_conection_serv();
		void set_rpl(std::string);
		void clear_rpl();

		//run orders
		int find_cmds();
		void cmds_register(User &);
		void orders(User &);
		void trim_cmds();
		void parse_order();
		void run_order(User &);

		//orders
		void nick(User &);
		void user(User &);
		void modeUser(User &);
		void join(User &);
		void modeChannel(User &);
		void pass_cmd(User &);
		void invite(User &);
		void topic(User &);
		void kick(User &);
		void part(User &);
		void quit(User &);
		void privmsg(User &);

};

std::ostream& operator<<(std::ostream&, Server&); 

#endif