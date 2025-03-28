#include "Channel.hpp"

Channel::Channel() : _name("default"), _topicRestricted(false) 
{
    _topic = "";
    _inviteOnly = false;
    _password = "";
    _maxClients = -1;
}

Channel::Channel(const std::string& name) : _topicRestricted(false)
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

// Ajoutez ces méthodes :

void Channel::setTopic(const std::string& topic) {
    _topic = topic;
}

const std::string& Channel::getTopic() const {
    return _topic;
}

void Channel::setInviteOnly(bool inviteOnly) {
    _inviteOnly = inviteOnly;
}

bool Channel::getInviteOnly() const {
    return _inviteOnly;
}

void Channel::setPassword(const std::string& password) {
    _password = password;
}

const std::string& Channel::getPassword() const {
    return _password;
}

void Channel::setMaxClients(int maxClients) {
    _maxClients = maxClients;
}

int Channel::getMaxClients() const {
    return _maxClients;
}

void Channel::addInvited(const std::string& nickname) {
    if (!isInvited(nickname))
        _invited.push_back(nickname);
}

bool Channel::isInvited(const std::string& nickname) const {
    return std::find(_invited.begin(), _invited.end(), nickname) != _invited.end();
}

bool Channel::getTopicRestricted() const {
    return _topicRestricted;
}

void Channel::setTopicRestricted(bool restricted) {
    _topicRestricted = restricted;
}

void Channel::broadcast(const std::string& message) {
    for (size_t i = 0; i < _clients.size(); ++i) {
        send(_clients[i]->getFd(), message.c_str(), message.size(), 0);
    }
}

//                     Gestion des clients

void Channel::addClient(Client* client, const std::string& password) {
    // 1. Vérifier la limite d'utilisateurs (+l)
    if (_maxClients > 0 && _clients.size() >= static_cast<size_t>(_maxClients)) {
        send(client->getFd(), ":SERVER 471 #general :Cannot join channel (+l)\r\n", 46, 0);
        return;
    }

    // 2. Vérifier le mot de passe (+k) si fourni
    if (!_password.empty() && _password != password) {
        send(client->getFd(), ":SERVER 475 #general :Bad channel key\r\n", 40, 0);
        return;
    }

    // 3. Vérifier le mode invitation (+i)
    if (_inviteOnly && !isInvited(client->getNickname())) {
        send(client->getFd(), ":SERVER 473 #general :Invite-only channel\r\n", 43, 0);
        return;
    }

    // 4. Ajouter le client
    _clients.push_back(client);
    if (_clients.size() == 1) { // Premier client = OP
        _operators.push_back(client);
    }

    // Notification IRC standard
    std::string msg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost JOIN :" + _name + "\r\n";
    broadcast(msg);
}

void Channel::alertAll(const std::string& nickname, const std::string& msg) {
    std::string final = ":" + nickname + "!" + nickname + "@localhost " + msg + "\r\n";
    for (size_t i = 0; i < _clients.size(); i++) {
        if (_clients[i]->getNickname() != nickname) {
            send(_clients[i]->getFd(), final.c_str(), final.size(), 0);
        }
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