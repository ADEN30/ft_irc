#ifndef RPLY_HPP
#define RPLY_HPP
#include "Server.hpp"
#include "utilsTosend.hpp"

class Server;



#include <sstream>

template <typename T>
  std::string NumberToString ( T Number )
  {
     std::ostringstream ss;
     ss << Number;
     return ss.str();
  }


#define MAXCHANLEN 20
#define MAXNICKLEN 20
#define MAXUSERLEN 20
#define MAXTOPICLEN 510
#define MAXUSERCHANNEL 10


#define RED_TEXT "\033[1;31m"
#define GREEN_TEXT "\033[1;32m"
#define RESET_TEXT "\033[0m"

#define BEGIN_RPL(server) (":" + server->gethostname_srv())

//Registration
#define RPL_WELCOME(server, nick) (BEGIN_RPL(server) + " 001 " + nick + " :Welcome to the IRC Network, " + nick + "\r\n")
#define RPL_YOURHOST(server, nick) (BEGIN_RPL(server) + " 002 " + nick + " :Your host is Flower, running version 0.1\r\n")
#define RPL_CREATED(server, nick) (BEGIN_RPL(server) + " 003 " + nick + " :This server was created " + getstr_time() + "\r\n")
#define RPL_MYINFO(server, nick)  (BEGIN_RPL(server) + " 004 " + nick + " :Flower 0.1  iOr CHANMODE=itkol\r\n")
#define RPL_ISUPPORT(server, nick)  (BEGIN_RPL(server) + " 005 " + nick + " CHANMODES=o,k,l,ti CHANTYPES=# TARGMAX=NICK:1, PRIVMSG:1 :are supported by this server\r\n")
#define RPL_MOTD(server, nick)  (BEGIN_RPL(server) + " 005 " + nick + " :- Flower message of the day -\n" + \
"  _____   _ \n\
 |  ___| | |   ___   __      __   ___   _ __ \n\
 | |_    | |  / _ \\  \\ \\ /\\ / /  / _ \\ | '__|\n\
 |  _|   | | | (_) |  \\ V  V /  |  __/ | |   \n\
 |_|     |_|  \\___/    \\_/\\_/    \\___| |_| \r\n")

#define RPL_ENDOFMOTD(server, nick) (BEGIN_RPL(server) + " 376 " + nick + " :END of /MOTD command.\r\n")

//MODE
#define RPL_UMODEIS(server, nick, mode) (BEGIN_RPL(server) + " 221 " + nick + " :+" + mode + "\r\n") 
#define RPL_MODE(server, nick, signe, mode) (BEGIN_RPL(server) + " MODE " + nick + " :" + signe + mode + "\r\n") 
#define ERR_UMODEUNKNOWNFLAG(server, nick) (BEGIN_RPL(server) + " 501 " + nick + " :Unknown MODE flag\r\n") 
#define ERR_USERSDONTMATCH(server, nick) (BEGIN_RPL(server) + " 502 " + nick + " :Can't change mode for other users\r\n") 

//PING PONG
#define RPL_PING(server, nick, str) (BEGIN_RPL(server) + " PONG " + nick + " :" + str + "\r\n")
#define RPL_PONG(server, nick, str) (BEGIN_RPL(server) + " PING " + nick + " :" + str + "\r\n")

//Nick
#define RPL_NICK(server, oldnick, nick) (":" + oldnick + "!d@localhost" +  " NICK " + nick  + "\r\n")
#define ERR_NONICKNAMEGIVEN(server, nick) (BEGIN_RPL(server) + " 431 " + nick + " :No nickname given\r\n")
#define ERR_ERRONEUSNICKNAME(server, nick) (BEGIN_RPL(server) + " 432 " + nick + " :Erroneus nickname\r\n")
#define ERR_NICKNAMEINUSE(server, nick, newnick) (BEGIN_RPL(server) + " 433 " + nick + " :" + newnick + "\r\n")

// Channel
#define RPL_JOIN(server, nick, user, ip, channel) (":" + nick + "!" + user + "@" + ip + " JOIN " + channel + " " + nick +" :Real Name\r\n")
#define RPL_NAMREPLY(server, nick, channel,  str) (BEGIN_RPL(server) + " 353 " + nick + " = " + channel + " :" + str + "\r\n")
#define RPL_ENDOFNAMES(server, nick, channel) (BEGIN_RPL(server) + " 366 " + nick + " " + channel + " :End of /NAMES list"  + "\r\n")
#define ERR_NOSUCHCHANNEL(server, nick, channel) (BEGIN_RPL(server) + " 403 " + nick + " " + channel + " :No such channel\r\n")

//Channel mode
#define RPL_CHANNELMODEIS(server, nick, channel, modes, args) (BEGIN_RPL(server) + " 324 " + nick + " " + channel + " " + modes + args + "\r\n")
#define ERR_BADCHANNELKEY(server, nick, channel) (BEGIN_RPL(server) + " 475 " + nick + " " + channel + " :Cannot join channel (+k)\r\n") 
#define ERR_CHANNELISFULL(server, nick, channel) (BEGIN_RPL(server) + " 471 " + nick + " " + channel + " :Cannot join channel (+l)\r\n") 
#define ERR_INVITEONLYCHAN(server, nick, channel) (BEGIN_RPL(server) + " 473 " + nick + " " + channel + " :Cannot join channel (+i)\r\n") 
#define ERR_BANNEDFROMCHAN(server, nick, channel) (BEGIN_RPL(server) + " 474 " + nick + " " + channel + " :Cannot join channel (+b)\r\n") 
#define RPL_MODECHANNEL(nick, user, ip, channel, str)  (":" + nick + "!" + user + "@" + ip + " MODE " + channel + " :" + str + "\r\n")
#define RPL_MODECHANNELSERVEUR(server, channel, str)  (BEGIN_RPL(server) + " MODE " + channel + " :+" + str + "\r\n")
#define ERR_CHANOPRIVSNEEDED(server, nick, channel) (BEGIN_RPL(server) + " 482 " + nick + " " + channel + " :You'are not channel operator\r\n") 


//PASS
#define ERR_NEEDMOREPARAMS(nick, server) (BEGIN_RPL(server) + ": 461 " + nick + " :Not enough parameters\r\n")
#define ERR_ALREADYREGISTERED(nick, server) (BEGIN_RPL(server) + ": 462 " + nick + " :You may not reregister !\r\n")
#define ERR_PASSWDMISSMATCH(nick, server) (BEGIN_RPL(server) + ": 464 " + nick + " :Password incorrect !\r\n")

//Invite
#define ERR_NOTONCHANNEL(server, nick, channel) (BEGIN_RPL(server) + " 442 " + nick + " " + channel + " :You're not on that channel !\r\n")
#define ERR_USERONCHANNEL(server, nick, target, channel) (BEGIN_RPL(server) + " 443 " + nick + " " + target + " " + channel + " :is already on channel !\r\n")
#define RPL_INVITING(server, nick, target, channel) (BEGIN_RPL(server) + " 341 " + nick + " :" + target + " " + channel + "\r\n")
#define RPL_INVITEMSG(server, nick, user, ip, target, channel) (":"+ nick +"!" + user + "@" + ip + " INVITE " + target + " " + channel + "\r\n")

//TOPIC
#define RPL_NOTOPIC(server, nick, channel) (BEGIN_RPL(server) + " 331 " + nick + " " + channel + " :No topic is set\r\n")
#define RPL_TOPIC(server, nick, channel, topic) (BEGIN_RPL(server) + " 332 " + nick + " " + channel + " :" + topic + "\r\n")

//Kick
#define ERR_USERNOTINCHANNEL(server, nick, channel) (BEGIN_RPL(server) + " 441 " + nick + " " + channel + " :They aren't on that channel\r\n")
#define RPL_KICK(server, nick, user, ip, channel, target) (":"+ nick +"!" + user + "@" + ip + " KICK " + channel + " " + target + "\r\n")

//Part
#define RPL_PART(server, nick, user, ip, channel, context) (":"+ nick +"!" + user + "@" + ip + " PART " + channel + " :" + context + "\r\n")

//Quit
#define RPL_QUIT(nick, user, ip, context) (":"+ nick +"!" + user + "@" + ip + " QUIT :" + context + "\r\n")
#define ERROR_(context) (":ERROR :" + context + "\r\n")
 
 //Privmsg
 #define RPL_PRIVMSG(nick, user, ip, target, msg) (":"+ nick +"!" + user + "@" + ip + " PRIVMSG " + target + " :" + msg + "\r\n")
 #define ERR_NOSUCHNICK(server, nick, target) (BEGIN_RPL(server) + " 401 " + nick + " " + target + " :No such nick/channel\r\n")
#endif