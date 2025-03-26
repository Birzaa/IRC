#include "Commands.hpp"
#include "Server.hpp"
#include <sstream>
#include <algorithm>

void handleJoin(Server* server, Client* client, std::istringstream& iss)
{
    std::string channelName, key;
    iss >> channelName;
    iss >> key;

    if (channelName.empty() || channelName[0] != '#')
    {
        server->sendToClient(client->getFd(), "461 JOIN :Invalid channel name\r\n");
        return;
    }

    try
    {
        Channel& channel = server->getChannelManager().getChannel(channelName);
        
        if (channel.isInviteOnly() && !channel.isInvited(client->getNickname()))
        {
            server->sendToClient(client->getFd(), "473 " + channelName + " :Cannot join channel (+i)\r\n");
            return;
        }
        if (channel.hasKey() && channel.getKey() != key)
        {
            server->sendToClient(client->getFd(), "475 " + channelName + " :Cannot join channel (+k)\r\n");
            return;
        }
        if (channel.hasUserLimit() && channel.getUsers().size() >= channel.getUserLimit())
        {
            server->sendToClient(client->getFd(), "471 " + channelName + " :Cannot join channel (+l)\r\n");
            return;
        }
    }
    catch (const std::runtime_error& e)
    {
        server->getChannelManager().createChannel(channelName);
    }

    Channel& channel = server->getChannelManager().getChannel(channelName);
    channel.addUser(client->getNickname());
    
    if (channel.getUsers().size() == 1)
    {
        channel.addOperator(client->getNickname());
    }

    server->sendToClient(client->getFd(), ":" + client->getNickname() + " JOIN " + channelName + "\r\n");
    
    if (!channel.getTopic().empty())
    {
        server->sendToClient(client->getFd(), "332 " + client->getNickname() + " " + channelName + " :" + channel.getTopic() + "\r\n");
    }

    std::string namesList = "353 " + client->getNickname() + " = " + channelName + " :";
    const std::vector<std::string>& users = channel.getUsers();
    for (std::vector<std::string>::const_iterator it = users.begin(); it != users.end(); ++it)
    {
        namesList += (channel.isOperator(*it) ? "@" : "") + *it + " ";
    }
    namesList += "\r\n";
    server->sendToClient(client->getFd(), namesList);
    server->sendToClient(client->getFd(), "366 " + client->getNickname() + " " + channelName + " :End of /NAMES list\r\n");
}

void handlePart(Server* server, Client* client, std::istringstream& iss)
{
    std::string channelName, reason;
    iss >> channelName;
    std::getline(iss, reason);
    
    if (channelName.empty())
    {
        server->sendToClient(client->getFd(), "461 PART :Not enough parameters\r\n");
        return;
    }

    try
    {
        Channel& channel = server->getChannelManager().getChannel(channelName);
        
        if (!channel.hasUser(client->getNickname()))
        {
            server->sendToClient(client->getFd(), "442 " + channelName + " :You're not on that channel\r\n");
            return;
        }
        
        bool wasOperator = channel.isOperator(client->getNickname());
        channel.removeUser(client->getNickname());
        
        server->sendToClient(client->getFd(), ":" + client->getNickname() + " PART " + channelName + " :" + reason + "\r\n");
        
        if (wasOperator && !channel.getUsers().empty())
        {
            std::string newOperator = channel.getUsers()[0];
            channel.addOperator(newOperator);
            server->sendToClient(newOperator, "MODE " + channelName + " +o " + newOperator + "\r\n");
        }
        
        if (channel.isEmpty())
        {
            server->getChannelManager().deleteChannel(channelName);
        }
    }
    catch (const std::runtime_error& e)
    {
        server->sendToClient(client->getFd(), "403 " + channelName + " :No such channel\r\n");
    }
}

void handleKick(Server* server, Client* client, std::istringstream& iss)
{
    std::string channelName, targetNick, reason;
    iss >> channelName >> targetNick;
    std::getline(iss, reason);
    
    if (channelName.empty() || targetNick.empty())
    {
        server->sendToClient(client->getFd(), "461 KICK :Not enough parameters\r\n");
        return;
    }

    try
    {
        Channel& channel = server->getChannelManager().getChannel(channelName);
        Client* targetClient = server->getClientByNickname(targetNick);
        
        if (!targetClient)
        {
            server->sendToClient(client->getFd(), "401 " + targetNick + " :No such nick\r\n");
            return;
        }

        if (!channel.isOperator(client->getNickname()))
        {
            server->sendToClient(client->getFd(), "482 " + channelName + " :You're not channel operator\r\n");
            return;
        }
        
        if (!channel.hasUser(targetNick))
        {
            server->sendToClient(client->getFd(), "441 " + targetNick + " " + channelName + " :They aren't on that channel\r\n");
            return;
        }
        
        channel.removeUser(targetNick);
        server->sendToClient(client->getFd(), ":" + client->getNickname() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n");
        server->sendToClient(targetClient->getFd(), ":" + client->getNickname() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n");
    }
    catch (const std::runtime_error& e)
    {
        server->sendToClient(client->getFd(), "403 " + channelName + " :No such channel\r\n");
    }
}

void handleInvite(Server* server, Client* client, std::istringstream& iss)
{
    std::string targetNick, channelName;
    iss >> targetNick >> channelName;
    
    if (targetNick.empty() || channelName.empty())
    {
        server->sendToClient(client->getFd(), "461 INVITE :Not enough parameters\r\n");
        return;
    }

    try
    {
        Channel& channel = server->getChannelManager().getChannel(channelName);
        Client* targetClient = server->getClientByNickname(targetNick);
        
        if (!targetClient)
        {
            server->sendToClient(client->getFd(), "401 " + targetNick + " :No such nick\r\n");
            return;
        }

        if (!channel.isOperator(client->getNickname()))
        {
            server->sendToClient(client->getFd(), "482 " + channelName + " :You're not channel operator\r\n");
            return;
        }
        
        if (channel.hasUser(targetNick))
        {
            server->sendToClient(client->getFd(), "443 " + targetNick + " " + channelName + " :is already on channel\r\n");
            return;
        }
        
        channel.addInvited(targetNick);
        server->sendToClient(client->getFd(), "341 " + client->getNickname() + " " + targetNick + " " + channelName + "\r\n");
        server->sendToClient(targetClient->getFd(), ":" + client->getNickname() + " INVITE " + targetNick + " :" + channelName + "\r\n");
    }
    catch (const std::runtime_error& e)
    {
        server->sendToClient(client->getFd(), "403 " + channelName + " :No such channel\r\n");
    }
}


void handleTopic(Server* server, Client* client, std::istringstream& iss)
{
    std::string channelName, newTopic;
    iss >> channelName;
    std::getline(iss, newTopic);
    
    if (channelName.empty())
    {
        server->sendToClient(client->getFd(), "461 TOPIC :Not enough parameters\r\n");
        return;
    }

    try
    {
        Channel& channel = server->getChannelManager().getChannel(channelName);
        
        if (!channel.hasUser(client->getNickname()))
        {
            server->sendToClient(client->getFd(), "442 " + channelName + " :You're not on that channel\r\n");
            return;
        }
        
        if (newTopic.empty())
        {
            if (channel.getTopic().empty())
            {
                server->sendToClient(client->getFd(), "331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n");
            }
            else
            {
                server->sendToClient(client->getFd(), "332 " + client->getNickname() + " " + channelName + " :" + channel.getTopic() + "\r\n");
            }
            return;
        }
        
        if (channel.isTopicProtected() && !channel.isOperator(client->getNickname()))
        {
            server->sendToClient(client->getFd(), "482 " + channelName + " :You're not channel operator\r\n");
            return;
        }
        
        channel.setTopic(newTopic);
        server->sendToChannel(channelName, ":" + client->getNickname() + " TOPIC " + channelName + " :" + newTopic + "\r\n");
    }
    catch (const std::runtime_error& e)
    {
        server->sendToClient(client->getFd(), "403 " + channelName + " :No such channel\r\n");
    }
}

void handleMode(Server* server, Client* client, std::istringstream& iss)
{
    std::string target, mode, arg;
    iss >> target >> mode;
    iss >> arg;
    
    if (target.empty() || mode.empty())
    {
        server->sendToClient(client->getFd(), "461 MODE :Not enough parameters\r\n");
        return;
    }

    if (target[0] == '#')
    {
        try
        {
            Channel& channel = server->getChannelManager().getChannel(target);
            
            if (!channel.hasUser(client->getNickname()))
            {
                server->sendToClient(client->getFd(), "442 " + target + " :You're not on that channel\r\n");
                return;
            }
            
            if (!channel.isOperator(client->getNickname()))
            {
                server->sendToClient(client->getFd(), "482 " + target + " :You're not channel operator\r\n");
                return;
            }
            
            if (mode == "+i")
            {
                channel.setInviteOnly(true);
                server->sendToChannel(target, ":" + client->getNickname() + " MODE " + target + " +i\r\n");
            }
            else if (mode == "-i")
            {
                channel.setInviteOnly(false);
                server->sendToChannel(target, ":" + client->getNickname() + " MODE " + target + " -i\r\n");
            }
            else if (mode == "+t")
            {
                channel.setTopicProtected(true);
                server->sendToChannel(target, ":" + client->getNickname() + " MODE " + target + " +t\r\n");
            }
            else if (mode == "-t")
            {
                channel.setTopicProtected(false);
                server->sendToChannel(target, ":" + client->getNickname() + " MODE " + target + " -t\r\n");
            }
            else if (mode == "+k")
            {
                if (arg.empty())
                {
                    server->sendToClient(client->getFd(), "461 MODE :Not enough parameters\r\n");
                    return;
                }
                channel.setKey(arg);
                server->sendToChannel(target, ":" + client->getNickname() + " MODE " + target + " +k " + arg + "\r\n");
            }
            else if (mode == "-k")
            {
                channel.removeKey();
                server->sendToChannel(target, ":" + client->getNickname() + " MODE " + target + " -k\r\n");
            }
            else if (mode == "+o")
            {
                if (arg.empty())
                {
                    server->sendToClient(client->getFd(), "461 MODE :Not enough parameters\r\n");
                    return;
                }
                if (!channel.hasUser(arg))
                {
                    server->sendToClient(client->getFd(), "441 " + arg + " " + target + " :They aren't on that channel\r\n");
                    return;
                }
                channel.addOperator(arg);
                server->sendToChannel(target, ":" + client->getNickname() + " MODE " + target + " +o " + arg + "\r\n");
            }
            else if (mode == "-o")
            {
                if (arg.empty())
                {
                    server->sendToClient(client->getFd(), "461 MODE :Not enough parameters\r\n");
                    return;
                }
                channel.removeOperator(arg);
                server->sendToChannel(target, ":" + client->getNickname() + " MODE " + target + " -o " + arg + "\r\n");
            }
            else if (mode == "+l")
            {
                if (arg.empty())
                {
                    server->sendToClient(client->getFd(), "461 MODE :Not enough parameters\r\n");
                    return;
                }
                int limit = atoi(arg.c_str());
                if (limit <= 0)
                {
                    server->sendToClient(client->getFd(), "461 MODE :Invalid limit value\r\n");
                    return;
                }
                channel.setUserLimit(limit);
                server->sendToChannel(target, ":" + client->getNickname() + " MODE " + target + " +l " + arg + "\r\n");
            }
            else if (mode == "-l")
            {
                channel.removeUserLimit();
                server->sendToChannel(target, ":" + client->getNickname() + " MODE " + target + " -l\r\n");
            }
            else
            {
                server->sendToClient(client->getFd(), "472 " + mode + " :is unknown mode char to me\r\n");
            }
        }
        catch (const std::runtime_error& e)
        {
            server->sendToClient(client->getFd(), "403 " + target + " :No such channel\r\n");
        }
    }
    else
    {
        server->sendToClient(client->getFd(), "502 :Unknown MODE command\r\n");
    }
}