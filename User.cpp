#include "User.hpp"

User::User(int fd, Server &_serv) : _userMode(""), _so(fd), _regis(0), _server(&_serv){
    _fd.fd = fd;
    _fd.events = POLLIN;
}

User::~User()
{}

int User::getsock()
{
    return (_so);
}

int User::getregis()
{
    return (_regis);
}

void User::setregis()
{
	_regis = 1;
}

std::string& User::getstr()
{
    return (_str);
}

pollfd& User::getpollfd()
{
    return (_fd);
}

Server* User::getServer()
{
    return (_server);
}

bool User::getflag()
{
	return (_flag);
}


std::string User::get_name()
{
	return (_name);
}

void User::set_name(std::string &newname)
{
	_name = newname;
}

std::string User::get_username()
{
	return (_username);
}

std::vector<Chan*> User::get_channel()
{
	return (_chan);
}

void User::set_username(std::string &username)
{
	_username = username;
}

void User::set_realname(std::string &realname)
{
	_realname = realname;
}

std::string User::get_usermode()
{
	return (_userMode);
}

void User::set_usermode(std::string &usermode)
{
	_userMode = usermode;
}

void User::add_channel(Chan* channel)
{
	_chan.push_back(channel);
}

void User::setstr(char * str)
{
    _str.append(str);
}

void User::setflag(bool val)
{
	_flag = val;
}

char* User::getip()
{
	return (_ip);
}

void User::setip(char* ip)
{
	_ip = ip;
}

Chan* User::findchannel(Chan* channel)
{
	if (!channel)
		return (NULL);
	for (size_t i = 0; i < _chan.size(); i++)
	{
		if (channel->get_name() == _chan[i]->get_name())
			return (_chan[i]);
	}
	return (NULL);
	
}

void User::deleteChan(Chan* channel)
{
	std::vector<Chan*>::iterator _iterchan = _chan.begin();
	for (size_t i = 0; i < _chan.size(); i++)
	{
		if(_chan[i]->get_name() == channel->get_name())
		{
			_chan.erase(_iterchan);
			return ;
		}
		_iterchan++;
	}	
}

