#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "Client.hpp"
#include "ChannelManager.hpp"
#include "Server.hpp"

void sendIrcResponse(Client& client, const std::string& message);
void broadcastToChannel(Server& server, Channel& channel, const std::string& message, const std::string& excludeNick = "");

void handleJoinCommand(Server& server, ChannelManager& channelManager, Client& client, const std::string& channelName, const std::string& password = "");
void handlePartCommand(Server& server, ChannelManager& channelManager, Client& client, const std::string& channelName, const std::string& reason = "");
void handleKickCommand(Server& server, ChannelManager& channelManager, Client& client, const std::string& channelName, const std::string& targetUser, const std::string& reason = "");
void handleInviteCommand(Server& server, ChannelManager& channelManager, Client& client, const std::string& channelName, const std::string& targetUser);
void handleTopicCommand(Server& server, ChannelManager& channelManager, Client& client, const std::string& channelName, const std::string& newTopic = "");
void handleModeCommand(Server& server, ChannelManager& channelManager, Client& client, const std::string& channelName, const std::string& mode = "", const std::string& arg = "");

#endif