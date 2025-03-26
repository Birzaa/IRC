#include "Channel.hpp"
#include <algorithm>

Channel::Channel(const std::string& name) 
: name(name), inviteOnly(false), topicRestricted(false), userLimit(0)
{
}

void Channel::addUser(const std::string& user)
{
    if (std::find(users.begin(), users.end(), user) == users.end())
    {
        users.push_back(user);
    }
}

void Channel::removeUser(const std::string& user)
{
    users.erase(std::remove(users.begin(), users.end(), user), users.end());
    removeOperator(user);
}

bool Channel::hasUser(const std::string& user) const
{
    return std::find(users.begin(), users.end(), user) != users.end();
}

bool Channel::isEmpty() const
{
    return users.empty();
}

void Channel::setTopic(const std::string& newTopic)
{
    topic = newTopic;
}

std::string Channel::getTopic() const
{
    return topic;
}

void Channel::addOperator(const std::string& user)
{
    if (std::find(operators.begin(), operators.end(), user) == operators.end())
    {
        operators.push_back(user);
    }
}

void Channel::removeOperator(const std::string& user)
{
    operators.erase(std::remove(operators.begin(), operators.end(), user), operators.end());
}

bool Channel::isOperator(const std::string& user) const
{
    return std::find(operators.begin(), operators.end(), user) != operators.end();
}

const std::vector<std::string>& Channel::getUsers() const
{
    return users;
}

std::string Channel::getPassword() const
{
    return password;
}

size_t Channel::getUserLimit() const
{
    return userLimit;
}

void Channel::setMode(char mode, bool enable, const std::string& arg)
{
    switch (mode)
    {
        case 'i':
            inviteOnly = enable;
            break;
        case 't':
            topicRestricted = enable;
            break;
        case 'k':
            password = enable ? arg : "";
            break;
        case 'l':
            userLimit = enable ? atoi(arg.c_str()) : 0;
            break;
    }
}

bool Channel::checkInviteOnly() const
{
    return inviteOnly;
}

bool Channel::checkTopicRestricted() const
{
    return topicRestricted;
}

bool Channel::checkPassword(const std::string& pass) const
{
    return password.empty() || password == pass;
}

bool Channel::checkUserLimit() const
{
    return userLimit == 0 || users.size() < userLimit;
}

void Channel::addInvitedUser(const std::string& user)
{
    if (std::find(invitedUsers.begin(), invitedUsers.end(), user) == invitedUsers.end())
    {
        invitedUsers.push_back(user);
    }
}

bool Channel::isInvited(const std::string& user) const
{
    return std::find(invitedUsers.begin(), invitedUsers.end(), user) != invitedUsers.end();
}