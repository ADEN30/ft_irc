#ifndef USER_HPP
#define USER_HPP
#include <iostream>
#include <poll.h>
#include <list>
#include <cstring>
#include <map>
#include "Server.hpp"
#include "Rply.hpp"
#include <algorithm>
#include <cstdlib>
#include "Channel.hpp"
#include <vector>
#include "utilsTosend.hpp"

class Server;
class Chan;

class User
{
    private:
        std::string _name, _userMode, _username, _realname;
        int _so;
     	std::vector<Chan*> _chan;
        std::string _str;
		std::string _cmd;
        pollfd _fd;
        int _regis;
        Server * _server;
		bool _flag;
		char* _ip;

    public:
        User(int, Server &);
        ~User();
        int getsock();
        std::string& getstr();
        int getregis();
		void setregis();
        Server* getServer();
        pollfd& getpollfd();
		std::string get_name();
		std::string get_realname();
		std::string get_username();
		std::string get_usermode();
		std::vector<Chan*> get_channel();
		bool getflag();
		char* getip();

		//find somthing
		Chan* findchannel(Chan*);


		//delete something
		void deleteChan(Chan*);

        void setstr(char*);
		void set_name(std::string &);
		void set_username(std::string &);
		void set_realname(std::string &);
		void set_usermode(std::string &);
		void set_channel(Chan &);
		void setflag(bool);
		void setip(char*);
};



#endif