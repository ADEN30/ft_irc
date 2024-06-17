#include "Server.hpp"

std::string parseMode(char c, std::string newmode, Chan* channel)
{
	std::string _mode = channel->get_mode();
	std::string _rpl = "";
	for (size_t i = 0; i < newmode.size(); i++)
	{
		if(c == '+' &&  std::string("tklbio").find(newmode[i]) != std::string::npos)
		{	
			if (_mode.find_first_of(newmode[i]) == std::string::npos || std::string("lko").find(newmode[i]) != std::string::npos)
			{
				if (newmode[i] != 'o')
					_mode.push_back(newmode[i]);
				_rpl.push_back(newmode[i]);	
			}
		}
		else if (c == '-' && std::string("tklbio").find(newmode[i]) != std::string::npos)
		{
			if (_mode.find_first_of(newmode[i]) != std::string::npos)
			{
				if (_mode.size() < 2)
					_mode.clear();
				else
					_mode.erase(_mode.find_first_of(newmode[i]), 1);
				_rpl.push_back(newmode[i]);	
			}
		}
	}
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
			_indexofletter++;
			length_input++;
		}
		catch(std::string &e)
		{
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
				_here->delete_symboleOp(_cmdparse[length_input++]);
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
		}
	}
}

std::string rplLKO(std::string str, Chan* _here, char sign)
{
	std::string _rpl = "";
	size_t _indexofmode = 0;
	if (sign == '-')
		return (_rpl);
	while ((_indexofmode = str.find_first_of("lko", _indexofmode)) != std::string::npos)
	{	
		_rpl.push_back(' ');
		if (str[_indexofmode] == 'l')
			_rpl += NumberToString(_here->get_limuser());
		else if (str[_indexofmode] == 'k')
			_rpl += _here->get_password();
		else if (str[_indexofmode] == 'o')
			_rpl += _here->getnewop();
		_indexofmode++;
	}
	return (_rpl);
}

void Server::modeChannel(User &user)
{
	char _sign = '+';
	std::string _rpl = "";
	size_t _index = 0;
	Chan* _here = already_channel(to_upper(_cmdparse[1]));

	if (!(_here))
		throw( ERR_NOSUCHCHANNEL(this, user.get_name(), _cmdparse[1]));
	else if (_cmdparse.size() == 2)
		throw(RPL_CHANNELMODEIS(this, user.get_name(), _here->get_name(), _here->get_mode(), rplLKO(_here->get_mode(), _here, _sign)));
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
		_rpl += rplLKO(_cmdparse[2], _here, _sign);
		_index = _cmdparse[2].find_first_not_of("+-", ++_index);	
	}
	if (!_rpl.empty())
	{
		_here->send_msg_to(_sendfd, user.getpollfd().fd);
		set_rpl(RPL_MODECHANNEL(user.get_name(), user.get_username(), user.getip(), _here->get_name(), _rpl));
	}
}