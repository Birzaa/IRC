#include "ChannelManager.hpp"

ChannelManager::ChannelManager()
{
}

ChannelManager::~ChannelManager()
{
}

// Constructeur de copie
ChannelManager::ChannelManager(const ChannelManager& other) : channels(other.channels)
{
}

// Opérateur d'affectation
ChannelManager& ChannelManager::operator=(const ChannelManager& other)
{
    if (this != &other) // Vérifie l'auto-affectation
    {
        channels = other.channels;
    }
    return *this;
}

void ChannelManager::createChannel(const std::string& name)
{
    channels[name] = Channel(name);
}

void ChannelManager::deleteChannel(const std::string& name)
{
    channels.erase(name);
}

Channel& ChannelManager::getChannel(const std::string& name)
{
    std::map<std::string, Channel>::iterator it = channels.find(name);
    if (it == channels.end())
    {
        throw std::runtime_error("Channel not found");
    }
    return it->second;
}