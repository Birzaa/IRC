#include "Channel.hpp"

Channel::Channel() : _name("default"), _topicRestricted(false) 
{
    _topic = "";
    _inviteOnly = false;
    _password = "";
    _maxClients = -1;
    _creationTime = time(NULL);
}

Channel::Channel(const std::string& name) : _topicRestricted(false)
{
    this->_name = name;
    this->_topic = "";
    this->_inviteOnly = false;
    this->_password = "";
    this->_maxClients = -1;
    _creationTime = time(NULL);

}

Channel::~Channel()
{
}

// Ajoutez ces méthodes :

void Channel::setTopic(const std::string& topic) 
{
    _topic = topic;
}

const std::string& Channel::getTopic() const 
{
    return _topic;
}

void Channel::setInviteOnly(bool inviteOnly) 
{
    _inviteOnly = inviteOnly;
}

bool Channel::getInviteOnly() const 
{
    return _inviteOnly;
}

void Channel::setPassword(const std::string& password) 
{
    _password = password;
}

const std::string& Channel::getPassword() const 
{
    return _password;
}

void Channel::setMaxClients(int maxClients) 
{
    _maxClients = maxClients;
}

int Channel::getMaxClients() const 
{
    return _maxClients;
}

void Channel::addInvited(const std::string& nickname) 
{
    if (!isInvited(nickname))
        _invited.push_back(nickname);
}

bool Channel::isInvited(const std::string& nickname) const 
{
    return std::find(_invited.begin(), _invited.end(), nickname) != _invited.end();
}

bool Channel::getTopicRestricted() const 
{
    return _topicRestricted;
}

void Channel::setTopicRestricted(bool restricted) 
{
    _topicRestricted = restricted;
}

void Channel::broadcast(const std::string& message) 
{
    for (size_t i = 0; i < _clients.size(); ++i) 
{
        send(_clients[i]->getFd(), message.c_str(), message.size(), 0);
    }
}

void Channel::broadcast2(const std::string& message, Client* exclude) 
{
   for (size_t i = 0; i < _clients.size(); ++i) 
   {
        if (_clients[i] != exclude) 
        {
            send(_clients[i]->getFd(), message.c_str(), message.size(), 0);
        }
    }
}

//                     Gestion des clients

void Channel::addClient(Client* client, const std::string& password) 
{
    std::string nick = client->getNickname();
    std::string user = client->getUsername();
    std::string host = client->getHostname();

    // 1. Vérifier la limite d'utilisateurs (+l)
    if (_maxClients > 0 && _clients.size() >= static_cast<size_t>(_maxClients)) 
    {
        std::string msg = ":localhost 471 " + nick + " " + _name + " :Cannot join channel (+l)\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // 2. Vérifier le mot de passe (+k)
    if (!_password.empty() && _password != password) 
    {
        std::string msg = ":localhost 475 " + nick + " " + _name + " :Cannot join channel (+k)\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // 3. Vérifier le mode invitation (+i)
    if (_inviteOnly && !isInvited(nick)) 
    {
        std::string msg = ":localhost 473 " + nick + " " + _name + " :Cannot join channel (+i)\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // 4. Ajouter le client
    _clients.push_back(client);
    if (_clients.size() == 1) 
    { // Premier client = OP
        _operators.push_back(client);
    }

    // 5. Envoyer le message JOIN
    sendJoinReply(client);
}

void Channel::alertAll(const std::string& nickname, const std::string& msg)
{
    std::string final = ":" + nickname + "!" + nickname + "@localhost " + msg + "\r\n";
    for (size_t i = 0; i < _clients.size(); i++) 
    {
        if (_clients[i]->getNickname() != nickname) 
        {
            send(_clients[i]->getFd(), final.c_str(), final.size(), 0);
        }
    }
}

void Channel::removeClient(Client* client, std::string arg)
{
    std::vector<Client*>::iterator it = std::find(_clients.begin(), _clients.end(), client);
    if (it != _clients.end()) 
	{
		std::string nick = client->getNickname();
        std::string user = client->getUsername();
        std::string host = client->getHostname();
        _clients.erase(it);
        std::string partMsg = ":" + nick + "!" + user + "@" + host + arg + _name + "\r\n";
        broadcast(partMsg);
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
        std::string msg = ":localhost 482 " + kicker->getNickname() + " " + _name + 
                         " :You're not channel operator\r\n";
        send(kicker->getFd(), msg.c_str(), msg.size(), 0);        
        return;
    }

    std::vector<Client*>::iterator it = std::find(_clients.begin(), _clients.end(), target);
    if (it != _clients.end()) 
	{
        _clients.erase(it);
        std::string msg = ":" + kicker->getNickname() + " KICK " + _name + " " + target->getNickname() + " :" + reason + "\r\n";
    	alertAll(kicker->getNickname(), "kicked " + target->getNickname() + " from channel");    
    }
}

bool Channel::isMember(Client* client) const 
{
	return std::find(_clients.begin(), _clients.end(), client) != _clients.end();
}

Client* Channel::getClient(const std::string& nickname) const 
{
    for (std::vector<Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) 
    {
        if ((*it)->getNickname() == nickname) 
        {
            return *it;
        }
    }
    return NULL;
}


void Channel::sendIrcReply(int code, Client* client, const std::string& arg1, const std::string& arg2) 
{
    std::ostringstream oss;
    oss << ":localhost " << code << " " << client->getNickname();
    
    if (!arg1.empty())
        oss << " " << arg1;
    if (!arg2.empty())
        oss << " :" << arg2;
    
    oss << "\r\n";
    std::string msg = oss.str();
    send(client->getFd(), msg.c_str(), msg.size(), 0);
}


void Channel::sendJoinReply(Client* client) 
{
    // 1. Envoie le message JOIN à tous les membres du canal
    std::string joinMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() 
                        + " JOIN " + _name + "\r\n";
    broadcast(joinMsg);

    // 2. Construit la liste des nicks avec préfixes opérateurs
    std::string namesList;
    for (size_t i = 0; i < _clients.size(); ++i) 
    {
        if (isOperator(_clients[i])) 
        {
            namesList += "@"; // Préfixe opérateur
        }
        namesList += _clients[i]->getNickname() + " ";
    }

    // 3. Envoie les réponses IRC via la fonction utilitaire
    sendIrcReply(353, client, "= " + _name, namesList);  // RPL_NAMREPLY (liste des nicks)
    sendIrcReply(366, client, _name, "End of /NAMES list"); // RPL_ENDOFNAMES


    // 4. Envoie le topic si existant
    if (!_topic.empty()) 
    {
        sendIrcReply(332, client, _name, _topic);  // RPL_TOPIC
    }
    
    // // 5. Ajoute date de création du channel
    // std::ostringstream creationMsg;
    // creationMsg << _creationTime;  // Timestamp UNIX
    // sendIrcReply(329, client, _name, creationMsg.str());
}
