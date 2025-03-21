#include "ChannelManager.hpp"

void ChannelManager::createChannel(const std::string& name) 
{
    channels.insert(std::make_pair(name, Channel(name)));
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
        throw std::runtime_error("Canal non trouvé");
    }
    return it->second;
}