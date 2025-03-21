#include "Channel.hpp"
#include <algorithm>

Channel::Channel(const std::string& name) : name(name) {}

void Channel::addUser(const std::string& user) 
{
    users.push_back(user);
}

void Channel::removeUser(const std::string& user) 
{
    users.erase(std::remove(users.begin(), users.end(), user), users.end());
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
    operators.push_back(user);
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