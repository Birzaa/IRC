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

//                     Gestion des clients

void Channel::addClient(Client* client) {
    
     if (_inviteOnly && !isInvited(client->getNickname())) {
        send(client->getFd(), CYAN "~> Channel is invite-only (use /INVITE)\n" RESET, 47, 0);
        return;
    }

    // Vérifier la limite d'utilisateurs
    if (_maxClients > 0 && _clients.size() >= (size_t)_maxClients) {
        send(client->getFd(), CYAN "~> Channel is full\n" RESET, 24, 0);
        return;
    }
    
    // Vérifier le mode invitation seulement
    if (_inviteOnly && !isInvited(client->getNickname())) {
        send(client->getFd(), CYAN "~> Channel is invite-only\n" RESET, 36, 0);
        return;
    }

    
    // Vérifier si déjà dans le channel
    if (std::find(_clients.begin(), _clients.end(), client) != _clients.end()) {
        send(client->getFd(), CYAN "~> Already in the channel\n" RESET, 37, 0);
        return;
    }

    // Ajouter le client
    if (_clients.empty())
        _operators.push_back(client);
    _clients.push_back(client);
    
    // Retirer de la liste d'invités si nécessaire
    _invited.erase(std::remove(_invited.begin(), _invited.end(), client->getNickname()), _invited.end());
    
    send(client->getFd(), (MAGENTA "~> You joined the channel\n" RESET), 37, 0);
    alertAll(client->getNickname(), "joined the channel");
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