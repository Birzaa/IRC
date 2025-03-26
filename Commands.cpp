#include "Commands.hpp"
#include "Server.hpp"
#include <algorithm>
#include <sstream>
#include <cstdlib>

void sendIrcResponse(Client& client, const std::string& message)
{
    std::string formattedMsg = message + "\r\n";
    send(client.getFd(), formattedMsg.c_str(), formattedMsg.size(), 0);
}

void broadcastToChannel(Server& server, Channel& channel, const std::string& message, const std::string& excludeNick)
{
    const std::vector<std::string>& users = channel.getUsers();
    for (size_t i = 0; i < users.size(); ++i)
    {
        if (users[i] != excludeNick)
        {
            Client* target = server.getClientByNickname(users[i]);
            if (target)
            {
                sendIrcResponse(*target, message);
            }
        }
    }
}

void handleJoinCommand(Server& server, ChannelManager& channelManager, Client& client, const std::string& channelName, const std::string& password)
{
    try
    {
        Channel& channel = channelManager.getChannel(channelName);
        
        if (channel.checkInviteOnly() && !channel.isInvited(client.getNickname()))
        {
            sendIrcResponse(client, "473 " + client.getNickname() + " " + channelName + " :Cannot join channel (+i)");
            return;
        }
        
        if (!channel.checkPassword(password))
        {
            sendIrcResponse(client, "475 " + client.getNickname() + " " + channelName + " :Cannot join channel (+k)");
            return;
        }
        
        if (!channel.checkUserLimit())
        {
            sendIrcResponse(client, "471 " + client.getNickname() + " " + channelName + " :Cannot join channel (+l)");
            return;
        }
        
        channel.addUser(client.getNickname());
        if (channel.getUsers().size() == 1)
        {
            channel.addOperator(client.getNickname());
        }
        
        sendIrcResponse(client, ":" + client.getNickname() + " JOIN " + channelName);
        
        if (!channel.getTopic().empty())
        {
            sendIrcResponse(client, "332 " + client.getNickname() + " " + channelName + " :" + channel.getTopic());
        }
        
        std::string userList;
        const std::vector<std::string>& users = channel.getUsers();
        for (size_t i = 0; i < users.size(); ++i)
        {
            userList += (channel.isOperator(users[i]) ? "@" : "") + users[i] + " ";
        }
        sendIrcResponse(client, "353 " + client.getNickname() + " = " + channelName + " :" + userList);
        sendIrcResponse(client, "366 " + client.getNickname() + " " + channelName + " :End of /NAMES list");
    }
    catch (const std::exception& e)
    {
        if (channelName.empty() || (channelName[0] != '#' && channelName[0] != '&'))
        {
            sendIrcResponse(client, "403 " + client.getNickname() + " " + channelName + " :Invalid channel name");
            return;
        }
        
        channelManager.createChannel(channelName);
        Channel& newChannel = channelManager.getChannel(channelName);
        newChannel.addUser(client.getNickname());
        newChannel.addOperator(client.getNickname());
        
        sendIrcResponse(client, ":" + client.getNickname() + " JOIN " + channelName);
        sendIrcResponse(client, "331 " + client.getNickname() + " " + channelName + " :No topic is set");
    }
}

void handlePartCommand(Server& server, ChannelManager& channelManager, Client& client, const std::string& channelName, const std::string& reason)
{
    try
    {
        Channel& channel = channelManager.getChannel(channelName);
        if (!channel.hasUser(client.getNickname()))
        {
            sendIrcResponse(client, "442 " + client.getNickname() + " " + channelName + " :You're not on that channel");
            return;
        }

        bool wasOperator = channel.isOperator(client.getNickname());
        channel.removeUser(client.getNickname());
        
        sendIrcResponse(client, ":" + client.getNickname() + " PART " + channelName + " :" + reason);
        
        if (wasOperator && !channel.getUsers().empty())
        {
            bool hasOperator = false;
            const std::vector<std::string>& users = channel.getUsers();
            for (size_t i = 0; i < users.size(); ++i)
            {
                if (channel.isOperator(users[i]))
                {
                    hasOperator = true;
                    break;
                }
            }
            
            if (!hasOperator)
            {
                std::string newOp = channel.getUsers()[0];
                channel.addOperator(newOp);
                sendIrcResponse(client, "MODE " + channelName + " +o " + newOp);
            }
        }

        if (channel.isEmpty())
        {
            channelManager.deleteChannel(channelName);
        }
    }
    catch (const std::exception& e)
    {
        sendIrcResponse(client, "403 " + client.getNickname() + " " + channelName + " :No such channel");
    }
}

void handleKickCommand(Server& server, ChannelManager& channelManager, Client& client, const std::string& channelName, const std::string& targetUser, const std::string& reason)
{
    try
    {
        Channel& channel = channelManager.getChannel(channelName);
        
        if (!channel.isOperator(client.getNickname()))
        {
            sendIrcResponse(client, "482 " + client.getNickname() + " " + channelName + " :You're not channel operator");
            return;
        }
        
        if (!channel.hasUser(targetUser))
        {
            sendIrcResponse(client, "441 " + client.getNickname() + " " + targetUser + " " + channelName + " :They aren't on that channel");
            return;
        }
        
        channel.removeUser(targetUser);
        std::string kickMsg = ":" + client.getNickname() + " KICK " + channelName + " " + targetUser + " :" + reason;
        sendIrcResponse(client, kickMsg);
        
        Client* target = server.getClientByNickname(targetUser);
        if (target)
        {
            sendIrcResponse(*target, kickMsg);
        }
    }
    catch (const std::exception& e)
    {
        sendIrcResponse(client, "403 " + client.getNickname() + " " + channelName + " :No such channel");
    }
}

void handleInviteCommand(Server& server, ChannelManager& channelManager, Client& client, const std::string& channelName, const std::string& targetUser)
{
    try
    {
        Channel& channel = channelManager.getChannel(channelName);
        
        if (!channel.isOperator(client.getNickname()))
        {
            sendIrcResponse(client, "482 " + client.getNickname() + " " + channelName + " :You're not channel operator");
            return;
        }
        
        if (channel.hasUser(targetUser))
        {
            sendIrcResponse(client, "443 " + client.getNickname() + " " + targetUser + " " + channelName + " :is already on channel");
            return;
        }
        
        channel.addInvitedUser(targetUser);
        
        Client* target = server.getClientByNickname(targetUser);
        if (target)
        {
            sendIrcResponse(*target, ":" + client.getNickname() + " INVITE " + targetUser + " :" + channelName);
        }
        sendIrcResponse(client, "341 " + client.getNickname() + " " + targetUser + " " + channelName);
    }
    catch (const std::exception& e)
    {
        sendIrcResponse(client, "403 " + client.getNickname() + " " + channelName + " :No such channel");
    }
}

void handleTopicCommand(Server& server, ChannelManager& channelManager, Client& client, const std::string& channelName, const std::string& newTopic)
{
    try
    {
        Channel& channel = channelManager.getChannel(channelName);
        
        if (!channel.hasUser(client.getNickname()))
        {
            sendIrcResponse(client, "442 " + client.getNickname() + " " + channelName + " :You're not on that channel");
            return;
        }
        
        if (newTopic.empty())
        {
            if (channel.getTopic().empty())
            {
                sendIrcResponse(client, "331 " + client.getNickname() + " " + channelName + " :No topic is set");
            }
            else
            {
                sendIrcResponse(client, "332 " + client.getNickname() + " " + channelName + " :" + channel.getTopic());
            }
            return;
        }
        
        if (channel.checkTopicRestricted() && !channel.isOperator(client.getNickname()))
        {
            sendIrcResponse(client, "482 " + client.getNickname() + " " + channelName + " :You're not channel operator");
            return;
        }
        
        channel.setTopic(newTopic);
        broadcastToChannel(server, channel, ":" + client.getNickname() + " TOPIC " + channelName + " :" + newTopic);
    }
    catch (const std::exception& e)
    {
        sendIrcResponse(client, "403 " + client.getNickname() + " " + channelName + " :No such channel");
    }
}

void handleModeCommand(Server& server, ChannelManager& channelManager, Client& client, const std::string& channelName, const std::string& mode, const std::string& arg)
{
    try
    {
        Channel& channel = channelManager.getChannel(channelName);
        
        if (!channel.isOperator(client.getNickname()))
        {
            sendIrcResponse(client, "482 " + client.getNickname() + " " + channelName + " :You're not channel operator");
            return;
        }
        
        if (mode.empty())
        {
            std::string modes = "+";
            std::string modeParams;
            
            if (channel.checkInviteOnly())
            {
                modes += "i";
            }
            if (channel.checkTopicRestricted())
            {
                modes += "t";
            }
            if (!channel.getPassword().empty())
            {
                modes += "k";
                modeParams += " " + channel.getPassword();
            }
            if (channel.getUserLimit() > 0)
            {
                modes += "l";
                std::ostringstream oss;
                oss << channel.getUserLimit();
                modeParams += " " + oss.str();
            }
            
            sendIrcResponse(client, "324 " + client.getNickname() + " " + channelName + " " + modes + modeParams);
            return;
        }
        
        bool enable = (mode[0] == '+');
        char modeChar = mode[1];
        
        switch (modeChar)
        {
            case 'i':
            case 't':
                channel.setMode(modeChar, enable);
                break;
            case 'k':
                if (enable && arg.empty())
                {
                    sendIrcResponse(client, "461 " + client.getNickname() + " MODE :Not enough parameters");
                    return;
                }
                channel.setMode(modeChar, enable, arg);
                break;
            case 'l':
                if (enable)
                {
                    if (arg.empty())
                    {
                        sendIrcResponse(client, "461 " + client.getNickname() + " MODE :Not enough parameters");
                        return;
                    }
                    channel.setMode(modeChar, enable, arg);
                }
                else
                {
                    channel.setMode(modeChar, false);
                }
                break;
            case 'o':
                if (arg.empty())
                {
                    sendIrcResponse(client, "461 " + client.getNickname() + " MODE :Not enough parameters");
                    return;
                }
                if (enable)
                {
                    channel.addOperator(arg);
                }
                else
                {
                    channel.removeOperator(arg);
                }
                break;
            default:
                sendIrcResponse(client, "472 " + client.getNickname() + " " + std::string(1, modeChar) + " :is unknown mode char");
                return;
        }
        
        std::string fullMode = mode;
        if (!arg.empty() && (modeChar == 'k' || modeChar == 'o' || modeChar == 'l'))
        {
            fullMode += " " + arg;
        }
        broadcastToChannel(server, channel, ":" + client.getNickname() + " MODE " + channelName + " " + fullMode);
    }
    catch (const std::exception& e)
    {
        sendIrcResponse(client, "403 " + client.getNickname() + " " + channelName + " :No such channel");
    }
}