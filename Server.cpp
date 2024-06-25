#include "Server.hpp"

Server::Server(std::string pass, std::string port)
{
	_poll = NULL;
	_size_poll = 0;
	_pass = pass;
	_port = port;
	_findtwopoint = 0;
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
		throw (-1);
	}
	gethostname(_hostname, sizeof(_hostname));
}

void Server::make_sockserv()
{
	int en = 1;
	_sock_serv = socket(_host->ai_family, _host->ai_socktype, _host->ai_protocol);
	if (_sock_serv == -1)
	{
		throw -2;
	}
	setsockopt(_sock_serv, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en));
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
	sockaddr *_adrr = &_addr_cli;
	socklen_t _size_cli = sizeof(_addr_cli);
	_fdcli = accept(_sock_serv, &_addr_cli, &_size_cli);
	sockaddr_in* ip_access = (sockaddr_in*)_adrr;
	User _new(_fdcli, *this);
	_new.setip(inet_ntoa(ip_access->sin_addr));
	_users.insert(std::make_pair(_fdcli, _new));
	makepollfd_fds();
}

void Server::readfds_serv(int fd)
{
	std::string  _buffer;
	char _buff_read[_BUFF_SIZE];
	User* _us = findUser(fd);
	long _bytes_r = 0;
	_bytes_r = recv(fd, _buff_read, _BUFF_SIZE - 1, 0);
	if (_bytes_r == -1)
	{
		return ;
	}
	if (_bytes_r == 0)
	{
		std::vector<Chan*> _listchannels = _us->get_channel();
		for (size_t i = 0; i < _listchannels.size(); i++)
		{
			_listchannels[i]->send_msg_to(_sendfd, fd);
			set_rpl(RPL_PART(this, _us->get_name(), _us->get_username(), _us->getip(), _listchannels[i]->get_name(), std::string("the deconexion is weird !")));
			sendfds_serv();
			_sendfd.clear();
			_listchannels[i]->deleteUser(_us);
			_us->deleteChan(_listchannels[i]);

		}	
		_users.erase(fd);
		close(fd);
		makepollfd_fds();
	}
	_buff_read[_BUFF_SIZE - 1] = 0;
	if (_bytes_r > 0)
	{
		_bufferread.append(_buff_read);
		std::cout << RED_TEXT << ">>" << _bufferread << RESET_TEXT << std::endl;
		bzero(_buff_read, sizeof(_buff_read));	
	}
	orders(*_us);
}

void Server::sendfds_serv()
{
	for (size_t i = 0; i < _rpl.size(); i++)
	{
		for (size_t j = 0; j < _sendfd.size(); j++)
		{	
			if(_rpl[i].size() < 513)
			{
				if (send(_sendfd[j], _rpl[i].c_str(), _rpl[i].size(), 0) == -1)
					std::cout << "Error Send" << std::endl;
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
	if (_poll)
		delete _poll;
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
			return (&i->second);
		}
	}
	return (NULL);
}

User* Server::findUserbyname(std::string _name)
{
	for (_ituser i = _users.begin(); i != _users.end(); i++)
	{
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
	const char* tab[] = 
	{"NICK", "USER", "PING", "PONG", "JOIN", "MODE", "PASS", "INVITE", "TOPIC", "KICK", "PART", "QUIT", "PRIVMSG"};
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
			twopoint = 1;
			_findtwopoint = 1;
			_nparam++;
		}
		if (_cmd.find_first_of(' ', _nparam + 1) != std::string::npos && !twopoint)
			_cmdparse.push_back(_cmd.substr(_nparam + 1, _cmd.find_first_of(' ', _nparam + 1) - _nparam - 1));
		else
			_cmdparse.push_back(_cmd.substr(_nparam + 1));	
		_nparam = _cmd.find_first_of(' ', _nparam + 1);
	}
	_cmd.clear();
}

void Server::orders(User &user)
{
    size_t _end;

    {
		while ((_end = _bufferread.find("\r\n")) != std::string::npos)	
		{
  		    try
			{
				_findtwopoint = 0;
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
			set_rpl(RPL_PING(this, user.get_name(), _cmdparse[1]));
			break ;
		}
		case 3:
		{
			set_rpl(RPL_PONG(this, user.get_name(), _cmdparse[1]));
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
			break ;
		case 9:
			kick(user);
			break ;
		case 10: 
			part(user);
			break ;
		case 11:
			quit(user);
			break;
		case 12:
			privmsg(user);
			break ;
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
	std::string _invalid = "#: ";

	if (!user.getflag())
		return ;
	else if (_cmdparse.size() < 2 || _index == std::string::npos || isdigit(_cmdparse[1][1]))
		throw (ERR_NONICKNAMEGIVEN(this, user.get_name()));
    else if(_invalid.find_first_of(_cmdparse[1]) != std::string::npos)
		throw (ERR_ERRONEUSNICKNAME(this, user.get_name()));
	while (findUserbyname(_cmdparse[1]))
	{
		set_rpl(ERR_NICKNAMEINUSE(this, user.get_name(), _cmdparse[1]));
		_cmdparse[1].push_back('_');
	}
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
		throw(ERR_NEEDMOREPARAMS(user.get_name(), this));
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
		}
		else
		{
			if (!_channel->finduser(&user))
				_channel->add_user(&user);
			try {
				if (_channel->get_mode().find('l') != std::string::npos && _channel->get_limuser() > 0 && _channel->get_limuser() < _channel->get_mapuser().size())
				{
					throw (ERR_CHANNELISFULL(this, user.get_name(), _channelname));
				}
				else if (_channel->get_mode().find('i') != std::string::npos && !user.findchannel(_channel))
					throw (ERR_INVITEONLYCHAN(this, _channel->get_name(), _channelname));
				else if (_channel->get_mode().find('k') != std::string::npos && !(_channel->get_password().empty()) && _passwordchannel != _channel->get_password())
					throw (ERR_BADCHANNELKEY(this, user.get_name(), _channelname));
				else if (_channel->get_banUser().size() > 0 && _channel->findbannedUser(&user))
					throw(ERR_BANNEDFROMCHAN(this, user.get_name(), _channelname));
			}
			catch(std::string &e){
				_channel->deleteUser(&user);
				throw(e);
			}
		}
		if (!user.findchannel(_channel))
			user.add_channel(_channel);	
		set_rpl(RPL_JOIN(this, user.get_name(), user.get_username(), user.getip(), _channel->get_name()));
		_channel->send_msg_to(_sendfd, user.getpollfd().fd);
		if (_sendfd.size() > 1)
		{
			sendfds_serv();
			_sendfd.erase(_sendfd.begin() + 1);
		}
		if (!_channel->gettopic().empty())
			set_rpl(RPL_TOPIC(this, user.get_name(), _channel->get_name(), _channel->gettopic()));
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
	{
		std::cout << "erreur mdp: " << _cmdparse[1] << "\t" << _pass << std::endl;
		throw(ERR_PASSWDMISSMATCH(user.get_name(), this));
	}
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
	set_rpl(RPL_INVITEMSG(this, user.get_name(), user.get_username(), user.getip(), _cmdparse[1], _channel->get_name()));
	findUserbyname(_cmdparse[1])->add_channel(_channel);
}

void Server::topic(User &user)
{
	if (_cmdparse.size() < 2)
		throw(ERR_NEEDMOREPARAMS(user.get_name(), this));
	Chan* _channel = already_channel(to_upper(_cmdparse[1]));
	if (!_channel)
		throw(ERR_NOSUCHCHANNEL(this, user.get_name(), _cmdparse[1]));
	else if (!_channel->findoperator(&user) && _channel->get_mode().find('t') != std::string::npos)
		throw(ERR_CHANOPRIVSNEEDED(this, user.get_name(), _channel->get_name()));
	else if (!_channel->finduser(&user))
		throw(ERR_NOTONCHANNEL(this, user.get_name(), _channel->get_name()));
	if (_findtwopoint && _cmdparse.size() == 2)
		_channel->settopic(std::string(""));
	else if (_cmdparse.size() > 2)
		_channel->settopic(_cmdparse[2]);
	_channel->send_msg_to(_sendfd, user.getpollfd().fd);
	set_rpl(RPL_TOPIC(this, user.get_name(), _channel->get_name(), _channel->gettopic()));	
}

void Server::kick(User &user)
{
	size_t _indexname = 0;
	size_t _sizename = 0;
	size_t _lengthname = _cmdparse[2].size();
	Chan* _channel = already_channel(to_upper(_cmdparse[1]));
	std::string _nickname = "";
	if (_cmdparse.size() < 3)
		throw(ERR_NEEDMOREPARAMS(user.get_name(), this));
	else if (!_channel)
		throw(ERR_NOSUCHCHANNEL(this, user.get_name(), _cmdparse[1]));
	else if (!_channel->finduser(&user))
		throw(ERR_NOTONCHANNEL(this, user.get_name(), _channel->get_name()));
	else if (!_channel->findoperator(&user))
		throw(ERR_CHANOPRIVSNEEDED(this, user.get_name(), _channel->get_name()));
	while (_indexname < _lengthname)
	{
		if (_cmdparse[2].find(',', _indexname) != std::string::npos)
		{
			_sizename = _cmdparse[2].find(',', _indexname) - _indexname;
			_nickname = _cmdparse[2].substr(_indexname, _sizename);
			_indexname = _cmdparse[2].find(',', _indexname) + 1;
		}
		else
		{
			_sizename = _lengthname - _indexname;
			_nickname = _cmdparse[2].substr(_indexname, _sizename);			
			_indexname = _lengthname;
		}
		if (!_channel->finduser(findUserbyname(_nickname)))
		{
			if (_sendfd.size() > 1)
				_sendfd.erase(_sendfd.begin() + 1);
			set_rpl(ERR_USERNOTINCHANNEL(this, user.get_name(), _channel->get_name()));
			sendfds_serv();
		}
		else if (user.get_name() != _nickname)
		{
			User* _userkicked;
			_userkicked = findUserbyname(_nickname);
			set_rpl(RPL_KICK(this, user.get_name(), user.get_username(), user.getip(), _channel->get_name(), _nickname));
			_channel->send_msg_to(_sendfd, user.getpollfd().fd);
			sendfds_serv();
			_channel->deleteUser(_userkicked);
			_userkicked->deleteChan(_channel);
		}
	}

}


void Server::part(User & user)
{
	size_t _indexname = 0;
	size_t _sizename = 0;
	size_t _lengthname = _cmdparse[1].size();
	std::string _context = "";
	Chan* _channel;  
	std::string _nickname = "";
	if (_cmdparse.size() < 2)
		throw(ERR_NEEDMOREPARAMS(user.get_name(), this));

	if (_cmdparse.size() > 2)
		_context = _cmdparse[2];
	while (_indexname < _lengthname)
	{
		if (_cmdparse[1].find(',', _indexname) != std::string::npos)
		{
			_sizename = _cmdparse[1].find(',', _indexname) - _indexname;
			_nickname = _cmdparse[1].substr(_indexname, _sizename);
			_indexname = _cmdparse[1].find(',', _indexname) + 1;
		}
		else
		{
			_sizename = _lengthname - _indexname;
			_nickname = _cmdparse[1].substr(_indexname, _sizename);			
			_indexname = _lengthname;
		}
		_channel = already_channel(to_upper(_nickname));
		if (!_channel)
		{
			if (_sendfd.size() > 1)
				_sendfd.erase(_sendfd.begin() + 1);
			set_rpl(ERR_NOSUCHCHANNEL(this, user.get_name(), _cmdparse[1]));
		}
		else if (!_channel->finduser(&user))
		{
			if (_sendfd.size() > 1)
				_sendfd.erase(_sendfd.begin() + 1);
			set_rpl(ERR_NOTONCHANNEL(this, user.get_name(), _channel->get_name()));
		}
		else
		{
			set_rpl(RPL_PART(this, user.get_name(), user.get_username(), user.getip(), _channel->get_name(), _context));
			_channel->send_msg_to(_sendfd, user.getpollfd().fd);
			_channel->deleteUser(&user);
			user.deleteChan(_channel);
			if (_channel->get_mapuser().size() < 1)
				deleteChan(_channel);
		}
		sendfds_serv();
	}
}


void Server::deleteChan(Chan* channel)
{
	std::vector<Chan*>::iterator _iterchan = _chan.begin();
	for (size_t i = 0; i < _chan.size(); i++)
	{
		if(_chan[i]->get_name() == channel->get_name())
		{
			delete _chan[i];
			_chan.erase(_iterchan);
		}
		_iterchan++;
	}
}

void Server::quit(User &user)
{

	std::vector<Chan*> _channels = user.get_channel();
	if (_channels.size() > 0)
	{
		set_rpl(ERROR_(_cmdparse[1]));
		sendfds_serv();
		for (size_t i = 0; i < _channels.size(); i++)
		{
			_channels[i]->send_msg_to(_sendfd, user.getpollfd().fd);
			set_rpl(RPL_QUIT(user.get_name(), user.get_username(), user.getip(), _cmdparse[1]));
			sendfds_serv();
			_channels[i]->deleteUser(&user);
			if (_channels[i]->get_mapuser().size() == 0)
				deleteChan(_channels[i]);
			_sendfd.erase(_sendfd.begin(), _sendfd.end());
		}
	}
	close(user.getpollfd().fd);
	_users.erase(user.getpollfd().fd);
	makepollfd_fds();
}


void Server::privmsg(User &user)
{
	User* _target;
	Chan* _channel;

	if ( (_target = findUserbyname(_cmdparse[1])))
	{
		set_rpl(RPL_PRIVMSG(user.get_name(), user.get_username(), user.getip(), _cmdparse[1], _cmdparse[2]));
		_sendfd.push_back(_target->getpollfd().fd);
		_sendfd.erase(_sendfd.begin());
	}
	else if ((_channel = already_channel(to_upper(_cmdparse[1]))))
	{
		set_rpl(RPL_PRIVMSG(user.get_name(), user.get_username(), user.getip(), _cmdparse[1], _cmdparse[2]));
		_channel->send_msg_to(_sendfd, user.getpollfd().fd);
		_sendfd.erase(_sendfd.begin());
	}
	else
		set_rpl(ERR_NOSUCHNICK(this, user.get_name(), _cmdparse[1]));
	sendfds_serv();
}