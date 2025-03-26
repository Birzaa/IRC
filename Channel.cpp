#include "Channel.hpp"

Channel::Channel(std::string const& name)
: _name(name), _topic(""), _key(""), _inviteOnly(false), 
  _topicProtected(false), _userLimit(0)
{
}

Channel::~Channel()
{
}

// Getters
std::string Channel::getName() const
{
    return _name;
}

std::string Channel::getTopic() const
{
    return _topic;
}

std::string Channel::getKey() const
{
    return _key;
}

std::vector<std::string> Channel::getUsers() const
{
    return _users;
}

bool Channel::isInviteOnly() const
{
    return _inviteOnly;
}

bool Channel::isTopicProtected() const
{
    return _topicProtected;
}

size_t Channel::getUserLimit() const
{
    return _userLimit;
}

bool Channel::hasUser(std::string const& nickname) const
{
    for (std::vector<std::string>::const_iterator it = _users.begin(); 
         it != _users.end(); ++it)
    {
        if (*it == nickname)
            return true;
    }
    return false;
}

bool Channel::isOperator(std::string const& nickname) const
{
    for (std::vector<std::string>::const_iterator it = _operators.begin(); 
         it != _operators.end(); ++it)
    {
        if (*it == nickname)
            return true;
    }
    return false;
}

bool Channel::isInvited(std::string const& nickname) const
{
    for (std::vector<std::string>::const_iterator it = _invited.begin(); 
         it != _invited.end(); ++it)
    {
        if (*it == nickname)
            return true;
    }
    return false;
}

bool Channel::isEmpty() const
{
    return _users.empty();
}

bool Channel::hasKey() const
{
    return !_key.empty();
}

bool Channel::hasUserLimit() const
{
    return _userLimit > 0;
}

// Setters
void Channel::setTopic(std::string const& topic)
{
    _topic = topic;
}

void Channel::setKey(std::string const& key)
{
    _key = key;
}

void Channel::setInviteOnly(bool inviteOnly)
{
    _inviteOnly = inviteOnly;
}

void Channel::setTopicProtected(bool topicProtected)
{
    _topicProtected = topicProtected;
}

void Channel::setUserLimit(size_t limit)
{
    _userLimit = limit;
}

// User management
void Channel::addUser(std::string const& nickname)
{
    if (!hasUser(nickname))
        _users.push_back(nickname);
}

void Channel::removeUser(std::string const& nickname)
{
    for (std::vector<std::string>::iterator it = _users.begin(); 
         it != _users.end(); ++it)
    {
        if (*it == nickname)
        {
            _users.erase(it);
            break;
        }
    }
    removeOperator(nickname);
    removeInvited(nickname);
}

void Channel::addOperator(std::string const& nickname)
{
    if (!isOperator(nickname))
        _operators.push_back(nickname);
}

void Channel::removeOperator(std::string const& nickname)
{
    for (std::vector<std::string>::iterator it = _operators.begin(); 
         it != _operators.end(); ++it)
    {
        if (*it == nickname)
        {
            _operators.erase(it);
            break;
        }
    }
}

void Channel::addInvited(std::string const& nickname)
{
    if (!isInvited(nickname))
        _invited.push_back(nickname);
}

void Channel::removeInvited(std::string const& nickname)
{
    for (std::vector<std::string>::iterator it = _invited.begin(); 
         it != _invited.end(); ++it)
    {
        if (*it == nickname)
        {
            _invited.erase(it);
            break;
        }
    }
}

void Channel::removeKey()
{
    _key.clear();
}

void Channel::removeUserLimit()
{
    _userLimit = 0;
}