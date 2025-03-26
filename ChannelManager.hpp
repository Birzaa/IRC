#ifndef CHANNELMANAGER_HPP
#define CHANNELMANAGER_HPP

#include "Channel.hpp"
#include <map>
#include <stdexcept>

class ChannelManager {
private:
    std::map<std::string, Channel> channels;

    bool isValidChannelName(const std::string& name) const {
        return !name.empty() && (name[0] == '#' || name[0] == '&');
    }

public:
    void createChannel(const std::string& name);
    void deleteChannel(const std::string& name);
    Channel& getChannel(const std::string& name);
    bool channelExists(const std::string& name) const;
};

#endif