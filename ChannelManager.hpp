#ifndef CHANNELMANAGER_HPP
#define CHANNELMANAGER_HPP

#include "Channel.hpp"
#include <map>
#include <stdexcept>

class ChannelManager 
{
    private:
        std::map<std::string, Channel> channels;

    public:
        void createChannel(const std::string& name);
        void deleteChannel(const std::string& name);
        Channel& getChannel(const std::string& name);
};

#endif