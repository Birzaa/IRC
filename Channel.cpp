#include "Channel.hpp"

Channel::Channel() : _name("default") 
{
    _topic = "";
    _inviteOnly = false;
    _password = "";
    _maxClients = -1;
}

Channel::Channel(const std::string& name) 
{
	this->_name = name;
	this->_topic = "";
	this->_inviteOnly = false;
	this->_password = "";
	this->_maxClients = -1;
}

Channel::~Channel()
{
}


//                     Gestion des clients

void Channel::addClient(Client* client) 
{
    if (_maxClients > 0 && _clients.size() >= (size_t)_maxClients)
	{
		send(client->getFd(), CYAN "~> Channel is full\n" RESET, 24, 0);
        return;
    }
	
	std::vector<Client*>::iterator it;
	it = std::find(_clients.begin(), _clients.end(), client);
    if (it != _clients.end()) 
	{
		send(client->getFd(), CYAN "~> Already in the channel\n" RESET, 37, 0);
        return;
    }
	if (_clients.empty())
		_operators.push_back(client);
    _clients.push_back(client);
	send(client->getFd(), (MAGENTA "~> You joined the channel\n" RESET), 37, 0);
	alertAll(client->getNickname(), "joined the channel");
}

void Channel::alertAll(const std::string& nickname, const std::string& msg)
{
	std::string final = YELLOW "~> " + nickname + " : " + msg + "\n" RESET;
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i]->getNickname() != nickname)
			send(_clients[i]->getFd(), final.c_str(), final.size(), 0);
	}
}

void Channel::removeClient(Client* client)
{
    std::vector<Client*>::iterator it = std::find(_clients.begin(), _clients.end(), client);
    if (it != _clients.end()) 
	{
        _clients.erase(it);
        alertAll(client->getNickname(), "left the channel");
		_operators.erase(std::remove(_operators.begin(), _operators.end(), client), _operators.end());
    }
}
const std::vector<Client*>& Channel::getClients() const
{
    return _clients;
}


//                     Gestion des messages

void Channel::broadcastMessage(const std::string& sender, const std::string& message) 
{
    std::string formattedMsg = ":" + sender + " PRIVMSG " + _name + " :" + message + "\r\n";
    
    for (size_t i = 0; i < _clients.size(); i++) 
	{
        if (_clients[i]->getNickname() != sender) 
            send(_clients[i]->getFd(), formattedMsg.c_str(), formattedMsg.size(), 0);
    }
}

//                     Gestion des opérateurs

bool Channel::isOperator(Client* client) const 
{
    return std::find(_operators.begin(), _operators.end(), client) != _operators.end();
}

void Channel::addOperator(Client* client) 
{
    if (!isOperator(client))
        _operators.push_back(client);
}
    
void Channel::removeOperator(Client* client) 
{
    _operators.erase(std::remove(_operators.begin(), _operators.end(), client), _operators.end());
}

const std::vector<Client*>& Channel::getOperators() const 
{ 
	return _operators; 
}



void Channel::kickClient(Client* kicker, Client* target, const std::string& reason) 
{
    if (!isOperator(kicker)) 
	{
		std::string msg = CYAN + _name + " :You're not a channel operator\n" RESET;
        send(kicker->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    std::vector<Client*>::iterator it = std::find(_clients.begin(), _clients.end(), target);
    if (it != _clients.end()) 
	{
        _clients.erase(it);
        std::string msg = ":" + kicker->getNickname() + " KICK " + _name + " " + target->getNickname() + " :" + reason + "\r\n";
    	alertAll(kicker->getNickname(), "kicked " + target->getNickname() + " from channel");    }
}