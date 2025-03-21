#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "ChannelManager.hpp"

// Commandes de base
void handleJoinCommand(ChannelManager& channelManager, const std::string& channelName, const std::string& user);
void handlePartCommand(ChannelManager& channelManager, const std::string& channelName, const std::string& user);

// Commandes avancées
void handleKickCommand(ChannelManager& channelManager, const std::string& channelName, const std::string& user, const std::string& targetUser);
void handleInviteCommand(ChannelManager& channelManager, const std::string& channelName, const std::string& user, const std::string& targetUser);
void handleTopicCommand(ChannelManager& channelManager, const std::string& channelName, const std::string& user, const std::string& newTopic);
void handleModeCommand(ChannelManager& channelManager, const std::string& channelName, const std::string& user, const std::string& mode);

#endif

