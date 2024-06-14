#include "Channel.hpp"

Chan::Chan(User& _op, std::string _n, std::string password) : _name(_n), _operator(&_op)
{
	_users.insert(std::make_pair(&_op, std::string("@")));
	std::cout << "password in chan is: " << password << std::endl;
	_mode = "t";
	if (!password.empty())
	{
		_pswd = password;
		_mode.push_back('k');
	}
	_topic = "";
	_limuser = 0;
	std::cout << _op.get_name() << std::endl;
	std::cout << _users.begin()->first->get_name() << std::endl;
}

std::string Chan::get_name()
{
	return (_name);
}
User* Chan::get_operator()
{
	return (_operator);
}

std::string Chan::get_password()
{
	return (_pswd);
}

std::string Chan::get_mode()
{
	return (_mode);
}

size_t Chan::get_limuser()
{
	return (_limuser);
}

std::map<User*, std::string> Chan::get_mapuser()
{
	return (_users);
}

std::vector<User*> Chan::get_banUser()
{
	return (_banUser);
}

void Chan::set_mode(std::string str)
{
	_mode = str;
}

std::string Chan::getnewop()
{
	return (new_op);
}

void Chan::set_lk(char c, std::string str)
{	
	if (c == 'k')
	{
		_pswd = str;
		std::cout << "mot de passe: " << _pswd << std::endl;
	}
	else if (c == 'l')
	{
		_limuser = atoi(str.c_str());
		std::cout << "limite users: " << _limuser << std::endl;
		if (_limuser < _users.size())
			_limuser = _users.size();
	}
	else if(c == 'o')
	{
		for (std::map<User*, std::string>::iterator i = _users.begin(); i != _users.end(); i++)
		{
			if (i->first->get_name() == str && i->second.find('@') == std::string::npos)
			{
				i->second.push_back('@');
				new_op = i->first->get_name();
				return ;
			}
		}
		throw(std::string("user don't exist !"));
		
	}
	if (str.empty())
		throw (std::string("no argument with the mode"));
}

void Chan::delete_symboleOp(std::string nick)
{
	std::cout << "we delete the operator:" << nick << std::endl;
	for (std::map<User*, std::string>::iterator i = _users.begin(); i != _users.end(); i++)
	{
		std::cout << "name: " << i->first->get_name() << "\t power:" << i->second << "." << std::endl;
		if (i->first->get_name() == nick && i->second.find('@') != std::string::npos)
		{
			i->second = "";
			new_op = i->first->get_name();
			return ;
		}
	}
	throw (std::string("user don't exist !"));
}

void Chan::send_msg_to(std::vector<int> & fds, int sender)
{
	for (_it i = _users.begin(); i != _users.end(); i++)
	{
		if (i->first->getpollfd().fd != sender)
			fds.push_back(i->first->getpollfd().fd);
	}
	
}

void Chan::add_mode(char c)
{
	_mode.push_back(c);
}

void Chan::add_user(User* user)
{
	_users.insert(std::make_pair(user, std::string("")));
}

std::string Chan::string_for_rpl()
{
	std::string _str = "";
	if (_users.size() < 2)
	{
		_str += _users.begin()->second;
		_str += _users.begin()->first->get_name();
		return (_str);
	}
	for (_it i = _users.begin(); i != _users.end(); i++)
	{
		std::cout << i->second << "\n";
		if (i != _users.begin())
			_str += " ";
		_str += i->second;
		_str += i->first->get_name();
		}
	std::cout << "built rpl_string : " << _str << "." << std::endl;
	return (_str);
}

bool Chan::findbannedUser(User* user)
{
	for (size_t i = 0; i < _banUser.size(); i++)
	{
		if (user->get_name() == _banUser[i]->get_name())
		return (1);
	}
	return (0);
	
}

bool Chan::findoperator(User* user)
{
	for (_it i = _users.begin(); i != _users.end(); i++)
	{
		if (i->first->get_name() == user->get_name() && i->second.find('@') != std::string::npos)
			return (1);
	}
	return (0);	
}

bool Chan::finduser(User *user)
{
	if (!user)
		return (0);
	for (_it i = _users.begin(); i != _users.end(); i++)
	{
		if (i->first->get_name() == user->get_name())
			return (1);
	}
	return (0);	
}


std::string Chan::gettopic()
{
	return (_topic);
}

void Chan::settopic(std::string topic)
{
	_topic = topic;
}

void Chan::deleteUser(User* user)
{
	if (_users.size() < 2)
		_users.clear();
	for (_it i = _users.begin(); i != _users.end(); i++)
	{
		if (i->first->get_name() == user->get_name())
		{
			_users.erase(i->first);
			return ;
		}
	}
	
}