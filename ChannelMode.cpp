#include "Server.hpp"

std::string parseMode(char c, std::string newmode, Chan* channel)
{
	std::string _mode = channel->get_mode();
	std::string _rpl = "";
	for (size_t i = 0; i < newmode.size(); i++)
	{
		if(c == '+' &&  std::string("tklbio").find(newmode[i]) != std::string::npos)
		{	
			if (_mode.find_first_of(newmode[i]) == std::string::npos && newmode[i] != 'o')
				_mode.push_back(newmode[i]);
			_rpl.push_back(newmode[i]);	
		}
		else if (c == '-' && std::string("tklbio").find(newmode[i]) != std::string::npos)
		{
			if (_mode.size() < 2)
				_mode.clear();
			else if (_mode.find_first_of(newmode[i]) != std::string::npos)
				_mode.erase(_mode.find_first_of(newmode[i]), 1);
			_rpl.push_back(newmode[i]);	
		}
	}
	channel->set_mode(_mode);
	if (!_rpl.empty())
		_rpl.insert(_rpl.begin(), c);
	return(_rpl);
}

void parse_lk(std::vector<std::string> & _cmdparse, Chan *_here, char sign)
{
	size_t length_input = _cmdparse.size();
	size_t _indexofletter = 0;

	while (length_input > 3 && sign == '+')
	{
		try
		{
			_indexofletter = _cmdparse[2].find_first_of("lko", _indexofletter);	
			_here->set_lk(_cmdparse[2][_indexofletter], _cmdparse[--length_input]);
			_indexofletter++;
		}
		catch(std::string &e)
		{
			std::cout << e << std::endl;
			if (_cmdparse[2].size() > 1)
				_cmdparse[2].erase(_indexofletter, 1);
			else
				_cmdparse[2].clear();
		}
	}	
	while ((_indexofletter = _cmdparse[2].find_first_of("lko", _indexofletter)) != std::string::npos)
	{
		try
		{
			if (sign == '-' && _cmdparse[2][_indexofletter] == 'o' && _cmdparse.size() > 3)
				_here->delete_symboleOp(_cmdparse[--length_input]);
			else
				_here->set_lk(_cmdparse[2][_indexofletter], "");
			_indexofletter++;
		}
		catch (std::string &e)
		{
			std::cout << "delete of: " << _cmdparse[2][_indexofletter] << "\t" << e << std::endl;
			if (sign == '+' || (_cmdparse[2][_indexofletter] == 'o' && sign == '-'))
			{
				if (_cmdparse[2].size() > 1)
					_cmdparse[2].erase(_indexofletter, 1);
				else
					_cmdparse[2].clear();
			}	
			else
				_indexofletter++;
		}
	}
	std::cout << "last find to delete: " << _cmdparse[2].find_first_of("lko", _indexofletter) << std::endl;
}

std::string rplLKO(std::string str, Chan* _here)
{
	std::string _rpl = "";
	if (std::string("lko").find_first_of(str) == 0)
	{
		_rpl.push_back(' ');
		_rpl += NumberToString(_here->get_limuser());
		
	}
	if (std::string("lko").find_last_of(str) == 1)
	{
		_rpl.push_back(' ');
		_rpl += _here->get_password();
	}
	if (std::string("lko").find_last_of(str) == 2)
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
	size_t _index = 0;
	Chan* _here = already_channel(to_upper(_cmdparse[1]));

	std::cout << _cmdparse[1] << &_here << std::endl;
	if (!(_here))
		throw( ERR_NOSUCHCHANNEL(this, user.get_name(), _cmdparse[1]));
	else if (_cmdparse.size() == 2)
		throw(RPL_CHANNELMODEIS(this, user.get_name(), _here->get_name(), _here->get_mode(), rplLKO(_here->get_mode(), _here)));
	else if (!_here->findoperator(&user))
		throw (ERR_CHANOPRIVSNEEDED(this, user.get_name(), _here->get_name()));
	_index = _cmdparse[2].find_first_not_of("+-", _index);	
	while (_index != std::string::npos && _sign != '\0')
	{
		if (_cmdparse[2][_index - 1] == '+')	
			_sign = '+';
		else if (_cmdparse[2][_index - 1] == '-')	
			_sign = '-';
		else
			_sign = '\0';
		_cmdparse[2] = _cmdparse[2].substr(_index, _cmdparse[2].size() - _index);
		parse_lk(_cmdparse, _here, _sign);
		_rpl += parseMode(_sign, _cmdparse[2].substr(0, _cmdparse[2].size()), _here);
		_index = _cmdparse[2].find_first_not_of("+-", _index++);	
	}
	_rpl += rplLKO(_cmdparse[2], _here);
	if (!_rpl.empty())
	{
		_here->send_msg_to(_sendfd, user.getpollfd().fd);
		set_rpl(RPL_MODECHANNEL(user.get_name(), user.get_username(), user.getip(), _here->get_name(), _rpl));
	}
}