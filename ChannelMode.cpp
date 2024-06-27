#include "Server.hpp"

std::string parseMode(char c, std::string newmode, Chan* channel)
{
	std::string _mode = channel->get_mode();
	std::string _rpl = "";
	for (size_t i = 0; i < newmode.size(); i++)
	{
		if(c == '+' &&  std::string("tklio").find(newmode[i]) != std::string::npos)
		{	
			if (_mode.find(newmode[i]) == std::string::npos || std::string("lko").find(newmode[i]) != std::string::npos)
			{
				if (newmode[i] != 'o' && _mode.find(newmode[i]) == std::string::npos)
					_mode.push_back(newmode[i]);
				if (_rpl.find(newmode[i]) == std::string::npos)
					_rpl.push_back(newmode[i]);	
			}
		}
		else if (c == '-' && std::string("tklio").find(newmode[i]) != std::string::npos)
		{
			if (_mode.find_first_of(newmode[i]) != std::string::npos)
			{
				if (_mode.size() < 2)
					_mode.clear();
				else
					_mode.erase(_mode.find_first_of(newmode[i]), 1);
				_rpl.push_back(newmode[i]);	
			}
			else if (newmode[i] == 'o' && _rpl.find(newmode[i]) != std::string::npos)
				_rpl.push_back(newmode[i]);	
		}
	}
	std::cout << "ParseMode done: " << _mode << std::endl;
	channel->set_mode(_mode);
	if (!_rpl.empty())
		_rpl.insert(_rpl.begin(), c);
	return(_rpl);
}

void parse_lk(std::vector<std::string> & _cmdparse, Chan *_here, char sign)
{
	size_t length_input = 1;
	size_t _indexofletter = 0;

	while (sign == '+' && _cmdparse.size() > 2 + length_input && (_indexofletter = _cmdparse[2].find_first_of("lko", _indexofletter)) != std::string::npos)
	{
		try
		{
			_here->set_lk(_cmdparse[2][_indexofletter], _cmdparse[2 + length_input]);
			std::cout << _cmdparse[2 + length_input] << std::endl;
			_indexofletter++;
			length_input++;
		}
		catch(std::string &e)
		{
			if (_cmdparse[2].size() > 1)
				_cmdparse[2].erase(_indexofletter, 1);
			else
				_cmdparse[2].clear();
			std::cout << e << std::endl;
		}
	}	
	while ((_indexofletter = _cmdparse[2].find_first_of("lko", _indexofletter)) != std::string::npos)
	{
		try
		{
			if (sign == '-' && _cmdparse[2][_indexofletter] == 'o' && _cmdparse.size() > 3)
			{
				_here->delete_symboleOp(_cmdparse[2 + length_input++]);
			}
			else
				_here->set_lk(_cmdparse[2][_indexofletter], "\0");
			_indexofletter++;
		}
		catch (std::string &e)
		{
			if (sign == '+' || (_cmdparse[2][_indexofletter] == 'o' && sign == '-'))
			{
				if (_cmdparse[2].size() > 1)
					_cmdparse[2].erase(_indexofletter, 1);
				else
					_cmdparse[2].clear();
			}	
			else
				_indexofletter++;
			std::cout << e << std::endl;
		}
	}
}

std::string rplLKO(Chan* _here, std::string rpl)
{
	std::string _rpl = "";
	std::string _mode = _here->get_mode();
	std::cout << "Mode " << _mode << std::endl;
		if (_mode.find('l') != std::string::npos && rpl.find('l') != std::string::npos)
		{
			_rpl.push_back(' ');
			_rpl += NumberToString(_here->get_limuser());
		}
		if (_mode.find('k') != std::string::npos && rpl.find('k') != std::string::npos)
		{
			_rpl.push_back(' ');
			_rpl += _here->get_password();
		}
		if (!_here->getnewop().empty() && rpl.find('o') != std::string::npos)
		{
			_rpl.push_back(' ');
			_rpl += _here->getnewop();
		}
	return (_rpl);
}

void Server::modeChannel(User &user)
{
	char _sign = '+';
	std::string _rpl = "";
	std::string _tmp = "";
	size_t _index1 = 0;
	size_t _index2 = 0;
	Chan* _here = already_channel(to_upper(_cmdparse[1]));

	if (!(_here))
		throw( ERR_NOSUCHCHANNEL(this, user.get_name(), _cmdparse[1]));
	else if (_cmdparse.size() == 2)
		throw(RPL_CHANNELMODEIS(this, user.get_name(), _here->get_name(), _here->get_mode(), rplLKO(_here, std::string("lko"))));
	else if (!_here->findoperator(&user))
		throw (ERR_CHANOPRIVSNEEDED(this, user.get_name(), _here->get_name()));
	_tmp = _cmdparse[2];
	while ((_index1 = _tmp.find_first_not_of("+-", _index1)) != std::string::npos && _sign != '\0')
	{
		if (_tmp[_index1 - 1] == '+')	
			_sign = '+';
		else if (_tmp[_index1 - 1] == '-')	
			_sign = '-';
		else
			_sign = '\0';
		if ((_index2 = _tmp.find_first_of("+-", _index1)) != std::string::npos)	
		{
			std::cout << "find + or - and _index1: " << _index1 << std::endl;
			_cmdparse[2] = _tmp.substr(_index1, _index2 - 1);
			_tmp.erase(_index1 - 1, _index2);
			_index1 = 0;
		}
		else 
		{
			std::cout << "Don't find + or - and _index1: " << _index1 << std::endl;
			_cmdparse[2] = _tmp.substr(_index1);
			_tmp.clear();
		}
		std::cout << "_cmdparse[2]: " << _cmdparse[2] << std::endl;
		parse_lk(_cmdparse, _here, _sign);
		_rpl += parseMode(_sign, _cmdparse[2], _here);
	}
	_rpl += rplLKO(_here, _rpl);
	if (!_rpl.empty())
	{
		_here->send_msg_to(_sendfd, user.getpollfd().fd);
		set_rpl(RPL_MODECHANNEL(user.get_name(), user.get_username(), user.getip(), _here->get_name(), _rpl));
	}
}