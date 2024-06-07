#include "Server.hpp"

Server::Server(std::string pass, std::string port)
{
	_poll = NULL;
	_size_poll = 0;
	_pass = pass;
	_port = port;
	try
	{
		check_port();
		set_host();     	
		make_sockserv();
	}
	catch(int e)
	{
		std::cerr << e << '\n';
	}
}

Server::~Server()
{
	freeaddrinfo(_host);
	delete _poll;
	close(_sock_serv);
}

void Server::set_host()
{
	addrinfo _init;
	memset(&_init, 0, sizeof _init);
	_init.ai_family = AF_INET;
	_init.ai_flags = AI_PASSIVE;
	_init.ai_socktype = SOCK_STREAM;
	int _test = getaddrinfo(0, _port.c_str(), &_init, &_host);
	if (_test != 0)
	{
		std::cout << errno << std::strerror(errno) << std::endl;
		throw (-1);
	}
	gethostname(_hostname, sizeof(_hostname));
}

void Server::make_sockserv()
{
	_sock_serv = socket(_host->ai_family, _host->ai_socktype, _host->ai_protocol);
	if (_sock_serv == -1)
	{
		std::cout << errno << std::strerror(errno) << std::endl;
		throw -2;
	}
	User _serv(_sock_serv, *this);
	_users.insert(std::make_pair(_sock_serv, _serv));
	makepollfd_fds();
}

int Server::getsock_serv()
{
	return (_sock_serv);
}

addrinfo& Server::getaddrinfo_serv()
{
	return (*_host);
}

void Server::accept_conection_serv()
{
	int _fdcli;
	sockaddr _addr_cli;
	socklen_t _size_cli = sizeof(_addr_cli);
	_fdcli = accept(_sock_serv, &_addr_cli, &_size_cli);
	User _new(_fdcli, *this);
	_users.insert(std::make_pair(_fdcli, _new));
	std::cout << *this << std::endl;
	makepollfd_fds();
}

void Server::readfds_serv(int fd)
{
	std::string  _buffer;
	char _buff_read[_BUFF_SIZE];
	User* _us = findUser(fd);
	long _bytes_r = 1;
	_bytes_r = recv(fd, _buff_read, _BUFF_SIZE, 0);
	if (_bytes_r == -1)
	{
		std::cout << "i think the socket is closed" << std::endl;
		return ;
	}
	if (_bytes_r == 0)
	{
		_users.erase(fd);
		close(fd);
		makepollfd_fds();
	}
	_buff_read[_bytes_r] = 0;
	if (_bytes_r > 0)
	{
		_bufferread.append(_buff_read);
		bzero(_buff_read, _BUFF_SIZE);	
	}
	std::cout << RED_TEXT << ">>" << _bufferread << RESET_TEXT;
	orders(*_us);
}

void Server::sendfds_serv()
{
	for (size_t i = 0; i < _rpl.size(); i++)
	{
		for (size_t j = 0; j < _sendfd.size(); j++)
		{	
			if(_rpl[i].size() < 512)
			{
				send(_sendfd[j], _rpl[i].c_str(), _rpl[i].size(), 0);
				std::cout << GREEN_TEXT << "<<" << _rpl[i] << RESET_TEXT;
			}
		}
	}
	_rpl.clear();
}

void Server::run_serv()
{
	if (bind(_sock_serv, _host->ai_addr, _host->ai_addrlen) != 0)
	{
		std::cout << errno << std::strerror(errno) << std::endl;
		throw -1;
	}
	listen(_sock_serv, 20);
	while (1)
	{
		int status = poll(_poll, _size_poll, 2000);
		if(status  == 0 || status == -1)
			continue ;
		else if (getRevents() > 0)
		{
			int _fd = getRevents();
			 if (_fd == _sock_serv)
			 {
				accept_conection_serv();
			 }
			 else
				readfds_serv(_fd);
		}
	}
}

int Server::getRevents()
{
	for (size_t i = 0; i < _size_poll; i++)
	{
		if(_poll[i].revents == POLLIN)
		{
			return (_poll[i].fd);
		}
	}
	
	return (-1);
}

std::map<int, User>& Server::getUsers()
{
	return (_users);
}

std::string Server::gethostname_srv()
{
	return ((std::string)_hostname);
}


void Server::makepollfd_fds()
{
	int i = 0;
	_poll = new pollfd[_users.size()];
	for(_ituser it = _users.begin(); it != _users.end(); it++)
	{
		_poll[i]= it->second.getpollfd();
		i++;
	}
	_size_poll = i;
	
}

User* Server::findUser(int fd)
{
	for (_ituser i = _users.begin(); i != _users.end(); i++)
	{
		if (i->first == fd)
		{
			return (&(i->second));
		}
	}
	std::cout << "User not Found\n";
	throw ;
}

User* Server::findUserbyname(std::string _name)
{
	for (_ituser i = _users.begin(); i != _users.end(); i++)
	{
		std::cout << i->second.get_name() << " || " << _name << " = " << _name.compare(i->second.get_name()) << std::endl;
		if(!_name.compare(0, _name.size(), i->second.get_name()))
			return (&i->second);
	}
	return (NULL);
}

void Server::set_rpl(std::string _str)
{
	_rpl.push_back(_str);
}

void Server::clear_rpl()
{
	_rpl.clear();
}

void Server::set_channel(Chan* channel)
{
	_chan.push_back(channel);
}

 //check something

Chan* Server::already_channel(std::string str)
{
	for (size_t i = 0; i < _chan.size(); i++)
	{
		if (to_upper(_chan[i]->get_name()) == str)
			return (_chan[i]);
	}
	return (NULL);
	
}

void Server::check_port()
{
	for (size_t i = 0; i < _port.size(); i++)
	{
		if (!isdigit(_port[i]))
			throw(std::string("port is not a number"));
	}
}

//User
std::ostream& operator<<(std::ostream & f, Server &s) 
{
	std::map<int, User> _us = s.getUsers();
	  for (_ituser it = _us.begin();   it != _us.end(); it++)
	  {
		  f << (it->second.getsock()) << std::endl; 
	  }
	  return (f);
}



//commandes

int Server::find_cmds()
{
	const char* tab[] = {"NICK", "USER", "PING", "PONG", "JOIN", "MODE", "PASS", "INVITE", "TOPIC"};
	std::list<std::string> _cmds(tab, tab + sizeof(tab) / sizeof(char*) );
	std::list<std::string>::iterator _it;
	int i = 0;
	for (_it = _cmds.begin(); _it != _cmds.end(); _it++)
	{
		if (_cmdparse[0].compare(0, _cmdparse[0].size(), *_it) == 0)
			return (i);
		i++;
	}
	return (-1);
}


void Server::trim_cmds()
{
	size_t _len = 0;
	bool double_point = 0;
	for (size_t i = 0; i < _cmd.size(); i++)
	{
		_len = 0;
		if (_cmd[i] == ':')
			double_point = 1;
		while(isspace(_cmd[i++]))
			_len++;
		if (_len > 1 && !double_point)
			_cmd = _cmd.erase(i - _len, _len - 1);
	}
}


void Server::parse_order()
{
	size_t _nparam = _cmd.find_first_of(' ', 0);
	int twopoint = 0;
	if (_nparam != std::string::npos)
		_cmdparse.push_back(_cmd.substr(0, _nparam));
	else
		_cmdparse.push_back(_cmd.substr(0));
	while (_nparam != std::string::npos && !twopoint)
	{
		if (_cmd[_nparam + 1] == ':')
		{
			std::cout << "find two point" << std::endl;
			twopoint = 1;
			_nparam++;
		}
		if (_cmd.find_first_of(' ', _nparam + 1) != std::string::npos && !twopoint)
			_cmdparse.push_back(_cmd.substr(_nparam + 1, _cmd.find_first_of(' ', _nparam + 1) - _nparam - 1));
		else
			_cmdparse.push_back(_cmd.substr(_nparam + 1));	
		_nparam = _cmd.find_first_of(' ', _nparam + 1);
	}
}

void Server::orders(User &user)
{
    size_t _end;

    {
        try
		{
       		_end = _bufferread.find("\r\n");
       		if (_end == std::string::npos)
       		    throw(std::string("no \\r\\n"));
			_cmd = _bufferread.substr(0, _end);
			trim_cmds();
			parse_order();
			if (!user.getregis())
				cmds_register(user);
			else
				run_order(user);
        }
        catch (std::string &test)
        {
            std::cout << test << std::endl;
        }
		if (_end != std::string::npos)
			_bufferread.erase(0, _end + 2);
    } 
}


void Server::cmds_register(User &user)
{	
	_sendfd.push_back(user.getpollfd().fd);
	try
	{
		switch (find_cmds())
		{
		case 0:
	            nick(user);
				break;
	    case 1:
		{
	           this->user(user);
	            break ;	
		}
		case 6:
		{
			pass_cmd(user);
		}
		default:
			break;
		}
		if (user.getregis())
		{
			set_rpl(RPL_WELCOME(this, user.get_name()));
			set_rpl(RPL_YOURHOST(this, user.get_name()));
			set_rpl(RPL_CREATED(this, user.get_name()));
			set_rpl(RPL_MYINFO(this, user.get_name()));
			set_rpl(RPL_ISUPPORT(this, user.get_name()));
			set_rpl(RPL_MOTD(this, user.get_name()));
			set_rpl(RPL_ENDOFMOTD(this, user.get_name()));
		}
	}
	catch(std::string & e)
	{
		set_rpl(e);
	}
	sendfds_serv();
	_cmd.clear();
	_cmdparse.clear();
	_sendfd.clear();
}


void Server::run_order(User &user)
{
	_sendfd.push_back(user.getpollfd().fd);
	try
	{
		switch (find_cmds())
		{
		case 0:
		{
			nick(user);
			break ;
		}
		case 1:
		{
			break ;
		}
		case 2:
		{	
			set_rpl(RPL_PING(this, user.get_name(), _cmd.substr(0, 5)));
			break ;
		}
		case 3:
		{
			set_rpl(RPL_PONG(this, user.get_name(), _cmd.substr(0, 5)));
			break ;
		}
		case 4:
		{
			join(user);
			break ;
		}
		case 5:
		{
			if (_cmdparse[1][0] != '#')
				modeUser(user);
			else
				modeChannel(user);
			break ;
		}
		case 7:
			invite(user);
			break ;
		case 8:
			topic(user);
		default:
			break;
		}
		sendfds_serv();
		_cmd.clear();
		_cmdparse.clear();
		_sendfd.clear();
	}
	catch(std::string &what)
	{
		set_rpl(what);
		sendfds_serv();
		_cmd.clear();
		_cmdparse.clear();
		_sendfd.clear();
	}
}

void Server::nick(User &user)
{
    size_t _index = 0;
	std::string _valid = "\\[]{}";
	if (!user.getflag())
		return ;
	if (_cmdparse.size() < 2 || _index == std::string::npos || isdigit(_cmdparse[1][_index + 1]))
		throw (ERR_NONICKNAMEGIVEN(this, user.get_name()));
    while (++_index < _cmd.size())
    {
        if(!isalnum(_cmd[_index]) && std::find(_valid.begin(), _valid.end(), _cmd[_index]) != _valid.end())
			throw (ERR_ERRONEUSNICKNAME(this, user.get_name()));
    }
	if (findUserbyname(_cmdparse[1]))
		throw (ERR_NICKNAMEINUSE(this, user.get_name(), _cmdparse[1]));
	std::string _oldname = user.get_name();
	user.set_name(_cmdparse[1]);
    if (!user.get_name().empty() && !user.get_username().empty() && user.getregis() == 0)
    	user.setregis();
	else if (user.getregis())
	{
		set_rpl(RPL_NICK(this, _oldname, user.get_name()));
	}     
}

void Server::user(User &user)
{
	if (!user.getflag())
		return ;
	if (_cmdparse.size() < 5)
	{
		return ;
	}
	if(_cmdparse[1].size() > 0 && _cmdparse[4].size() > 0)
	{
		user.set_username(_cmdparse[1]);
		user.set_realname(_cmdparse[4]);
	}
	if(!user.get_name().empty() && !user.get_username().empty())
	{
		user.setregis();
	}
}

void Server::modeUser(User &user)
{
	size_t _index = 0;
	size_t j = 2;
	std::string _mode = user.get_usermode();
	int	_signe = 1;
	if (_cmdparse[1].compare(0, user.get_name().size(), user.get_name()) != 0)
		throw ERR_USERSDONTMATCH(this, user.get_name());
	if (_cmdparse.size() < 3)
		throw RPL_UMODEIS(this, user.get_name(), user.get_usermode());
	while (_cmdparse.size() > j)
	{
		if (_cmdparse[j][_index] == '+')
			_signe = 1;
		else if (_cmdparse[j][_index] == '-' && _signe != -1)
			_signe *= -1;
		else if (!isspace(_cmdparse[j][_index]) && !(std::string("+-").find(_cmdparse[j][_index])) && !isalpha(_cmdparse[j][_index]))
			_signe = 0;
		if (_signe == 0)
			throw (std::string("PB syntaxe"));
		while(isalpha(_cmdparse[j][++_index]) && _index < _cmdparse[j].size())
		{
			std::cout << _cmdparse[j][_index] << std::endl;
			if (std::string("irO").find(_cmdparse[j][_index]) != std::string::npos)
			{
				if (_signe == 1 && _mode.find(_cmdparse[j][_index]) == std::string::npos)
				{
					_mode.push_back(_cmdparse[j][_index]);
					set_rpl(RPL_MODE(this, user.get_name(), "+", _cmdparse[j].substr(_index, 1)));
				}
				else if (_signe == -1 && _mode.find(_cmdparse[j][_index]) != std::string::npos)
				{
					if (_mode.size() == 1)
						_mode.clear();
					else
						_mode.erase(_mode.find(_cmdparse[j][_index]),_mode.find(_cmdparse[j][_index]));
					set_rpl(RPL_MODE(this, user.get_name(), "-", _cmdparse[j].substr(_index, 1)));
				}
			}
			else
				set_rpl(ERR_UMODEUNKNOWNFLAG(this, user.get_name()));
		}
		if (_index >= _cmdparse[j].size())
		{
			j++;
			_index = 0;
		}
	}
	user.set_usermode(_mode);
}

void Server::join(User &user)
{
	if (_cmdparse.size() > 3)
		throw (std::string("too much parameter"));
	std::string _channelname = "";
	std::string _passwordchannel = "";
	size_t _indexchannelname = 0;
	size_t _indexpaswordchannel = 0;
	size_t _cmdlen = _cmdparse[1].length();
	while (_indexchannelname < _cmdlen)	
	{
		if ( _cmdparse[1].find_first_of(',', _indexchannelname) != std::string::npos)
		{
			_channelname = _cmdparse[1].substr(_indexchannelname, _cmdparse[1].find_first_of(',', _indexchannelname) - _indexchannelname);
			if (_cmdparse[1].size() > 1 && !_cmdparse[2].empty() && _cmdparse[2].find_first_of(',', _indexpaswordchannel) != std::string::npos)
			{
				_passwordchannel = _cmdparse[2].substr(_indexpaswordchannel, _cmdparse[2].find_first_of(',', _indexpaswordchannel) - _indexpaswordchannel);
				_indexpaswordchannel = _cmdparse[2].find_first_of(',', _indexpaswordchannel) + 1;
			}
			_indexchannelname =  _cmdparse[1].find_first_of(',', _indexchannelname) + 1;
		}
		else
		{
			_channelname = _cmdparse[1].substr(_indexchannelname, _cmdparse[1].length() - _indexchannelname);
			std::cout << "_cmdparse[2]: " << _cmdparse[2] << std::endl;
			if(_cmdparse.size() == 3)
				_passwordchannel = _cmdparse[2].substr(_indexpaswordchannel, _cmdparse[2].length() - _indexpaswordchannel);
			else
				_passwordchannel.clear();
			_indexchannelname = _cmdparse[1].length();
		}
		if (_channelname[0]  != '#')
			throw (ERR_NOSUCHCHANNEL(this, user.get_name(), _channelname));
		Chan *_channel = already_channel(to_upper(_channelname));
		if (!_channel)
		{
			_channel = new Chan(user, _channelname, _passwordchannel);
			_chan.push_back(_channel);
			user.set_channel(*_channel);	
		}
		else
		{
			if (_channel->get_limuser() > 0 && _channel->get_limuser() < _channel->get_mapuser().size() + 1)
				throw (ERR_CHANNELISFULL(this, user.get_name(), _channelname));
			else if (_channel->get_mode().find('i') != std::string::npos)
				throw (ERR_INVITEONLYCHAN(this, _channel->get_name(), _channelname));
			else if (_channel->get_mode().find('k') != std::string::npos && !(_channel->get_password().empty()) && _passwordchannel != _channel->get_password())
				throw (ERR_BADCHANNELKEY(this, user.get_name(), _channelname));
			else if (_channel->get_banUser().size() > 0 && _channel->findbannedUser(&user))
				throw(ERR_BANNEDFROMCHAN(this, user.get_name(), _channelname));
			_channel->add_user(&user);
		}
		set_rpl(RPL_JOIN(this, user.get_name(), _channelname));
		_channel->send_msg_to(_sendfd, user.getpollfd().fd);
		if (_sendfd.size() > 1)
		{
			sendfds_serv();
			_sendfd.erase(_sendfd.begin() + 1);
		}
		set_rpl(RPL_MODECHANNELSERVEUR(this, _channel->get_name(), _channel->get_mode()));
		set_rpl(RPL_NAMREPLY(this, user.get_name(), _channel->get_name(), _channel->string_for_rpl()));
		set_rpl(RPL_ENDOFNAMES(this, user.get_name(), _channel->get_name()));	
		sendfds_serv();
	}
}


void Server::pass_cmd(User &user)
{
	if (_cmdparse.size() < 2)
		throw(ERR_NEEDMOREPARAMS(user.get_name(), this));
	if (user.getregis() == 1)
		throw(ERR_ALREADYREGISTERED(user.get_name(), this));
	if (_cmdparse[1] != _pass)
		throw(ERR_PASSWDMISSMATCH(user.get_name(), this));
	user.setflag(1);

}

void Server::invite(User &user)
{
	if (_cmdparse.size() < 3)
		throw(ERR_NEEDMOREPARAMS(user.get_name(), this));
	Chan* _channel = already_channel(to_upper(_cmdparse[2]));
	if (!_channel)
		throw(ERR_NOSUCHCHANNEL(this, user.get_name(), _cmdparse[2]));
	else if (!_channel->findoperator(&user) && _channel->get_mode().find('i') != std::string::npos)
		throw(ERR_CHANOPRIVSNEEDED(this, user.get_name(), _channel->get_name()));
	else if (!_channel->finduser(&user))
		throw(ERR_NOTONCHANNEL(this, user.get_name(), _channel->get_name()));
	else if(_channel->finduser(findUserbyname(_cmdparse[1])))
		throw(ERR_USERONCHANNEL(this, user.get_name(), _cmdparse[1], _channel->get_name()));
	set_rpl(RPL_INVITING(this, user.get_name(), _cmdparse[1], _channel->get_name()));
	sendfds_serv();
	_sendfd.erase(_sendfd.begin());
	_sendfd.push_back(findUserbyname(_cmdparse[1])->getpollfd().fd);
	set_rpl(RPL_INVITEMSG(this, user.get_name(), _cmdparse[1], _channel->get_name()));
}

void Server::topic(User &user)
{
	if (_cmdparse.size() < 3)
		throw(ERR_NEEDMOREPARAMS(user.get_name(), this));
	Chan* _channel = already_channel(to_upper(_cmdparse[1]));
	if (!_channel)
		throw(ERR_NOSUCHCHANNEL(this, user.get_name(), _cmdparse[1]));
	else if (!_channel->findoperator(&user) && _channel->get_mode().find('i') != std::string::npos)
		throw(ERR_CHANOPRIVSNEEDED(this, user.get_name(), _channel->get_name()));
	else if (!_channel->finduser(&user))
		throw(ERR_NOTONCHANNEL(this, user.get_name(), _channel->get_name()));
	else if (_channel->gettopic().empty())
		throw(RPL_NOTOPIC(this, user.get_name(), _channel->get_name()));
	set_rpl(RPL_TOPIC(this, user.get_name(), _channel->get_name(), _cmdparse[2]));
	
}