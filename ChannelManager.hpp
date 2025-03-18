#ifndef CHANNEL_MANAGER_HPP
#define CHANNEL_MANAGER_HPP

#include <map>
#include <stdexcept> // Pour std::runtime_error
#include "Channel.hpp"

class ChannelManager 
{
    private:
        std::map<std::string, Channel> channels;
    
    public:
        ChannelManager();
        ~ChannelManager();
        ChannelManager(const ChannelManager& other); // Constructeur de copie
        ChannelManager& operator=(const ChannelManager& other); // Opérateur d'affectation

        void createChannel(const std::string& name);
        void deleteChannel(const std::string& name);
        Channel& getChannel(const std::string& name); // Retourne une référence
};

#endif