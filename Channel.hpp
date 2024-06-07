#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include "User.hpp"
#include <map>
#include <vector>

class User;

class Chan
{
	private:
		std::map<User*, std::string> _users;
		typedef	std::map<User*, std::string>::iterator _it;
		std::vector<User*> _banUser;
		std::string _name;
		std::string _topic;
		std::string _pswd;
		size_t _limuser;
		std::string _mode;
		User* _operator;
		std::string new_op;

	public:	
		Chan(User&, std::string, std::string);
		
		//getteur
		std::string get_name();
		User* get_operator();
		std::string get_password();
		std::string get_mode();
		size_t	get_limuser();
		std::map<User*, std::string> get_mapuser();
		std::vector<User*> get_banUser();
		std::string getnewop();
		std::string gettopic();

		//setteur
		void set_mode(std::string);
		void set_lk(char, std::string);
		void add_mode(char);
		void send_msg_to(std::vector<int>&, int);

		void add_user(User*);
		void addoperator(User*);
		std::string string_for_rpl();
		std::string rpl_mode();

		//find something
		bool findbannedUser(User*);
		bool findoperator(User*);
		bool finduser(User*);
};


#endif