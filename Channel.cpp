#include "Channel.hpp"
#include <algorithm> // Pour std::find et std::remove

Channel::Channel() : name(""), users(), operators(), topic("")
{
}

Channel::~Channel()
{
}

Channel::Channel(const std::string& name) : name(name), users(), operators(), topic("")
{
}

// Constructeur de copie
Channel::Channel(const Channel& other)
    : name(other.name), users(other.users), operators(other.operators), topic(other.topic)
{
}

// Opérateur d'affectation
Channel& Channel::operator=(const Channel& other)
{
    if (this != &other) // Vérifie l'auto-affectation
    {
        name = other.name;
        users = other.users;
        operators = other.operators;
        topic = other.topic;
    }
    return *this;
}

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

void Channel::setTopic(const std::string& newTopic)
{
    topic = newTopic;
}

std::string Channel::getTopic() const
{
    return topic;
}

// Gestion des opérateurs
void Channel::addOperator(const std::string& user)
{
    if (!isOperator(user))
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