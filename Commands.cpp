#include "Commands.hpp"
#include <iostream>
#include <stdexcept>

// JOIN: Rejoindre un canal
void handleJoinCommand(ChannelManager& channelManager, MockServer& server, const std::string& channelName, const std::string& user) 
{
    try 
    {
        Channel& channel = channelManager.getChannel(channelName);
        channel.addUser(user);

        // Si le canal est vide avant d'ajouter l'utilisateur, le définir comme opérateur
        if (channel.getUsers().size() == 1) 
        {
            channel.addOperator(user);
        }

        server.sendMessage(user, "Vous avez rejoint le canal " + channelName);
        std::cout << user << " a rejoint le canal " << channelName << std::endl;
    } 
    catch (const std::runtime_error& e) 
    {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }
}

// PART: Quitter un canal
void handlePartCommand(ChannelManager& channelManager, MockServer& server, const std::string& channelName, const std::string& user) 
{
    try 
    {
        Channel& channel = channelManager.getChannel(channelName);
        bool wasOperator = channel.isOperator(user); // Vérifie si l'utilisateur qui part est un opérateur
        channel.removeUser(user);
        server.sendMessage(user, "Vous avez quitté le canal " + channelName);
        std::cout << user << " a quitté le canal " << channelName << std::endl;

        // Si l'utilisateur qui part était un opérateur, promouvoir un nouvel opérateur
        if (wasOperator && !channel.getUsers().empty()) 
        {
            std::string newOperator = channel.getUsers()[0]; // Prend le premier utilisateur de la liste
            channel.addOperator(newOperator);
            server.sendMessage(newOperator, "Vous êtes maintenant opérateur du canal " + channelName);
            std::cout << newOperator << " est maintenant opérateur du canal " << channelName << std::endl;
        }

        if (channel.isEmpty()) 
        {
            channelManager.deleteChannel(channelName);
        }
    } 
    catch (const std::runtime_error& e) 
    {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }
}

// KICK: Expulser un utilisateur du canal
void handleKickCommand(ChannelManager& channelManager, MockServer& server, const std::string& channelName, const std::string& user, const std::string& targetUser) 
{
    try 
    {
        Channel& channel = channelManager.getChannel(channelName);
        if (channel.isOperator(user)) 
        {
            channel.removeUser(targetUser);
            server.sendMessage(targetUser, "Vous avez été expulsé du canal " + channelName + " par " + user);
            std::cout << targetUser << " a été expulsé du canal " << channelName << " par " << user << std::endl;
        } 
        else 
        {
            std::cerr << "Erreur : " << user << " n'a pas les permissions nécessaires." << std::endl;
        }
    } 
    catch (const std::runtime_error& e) 
    {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }
}

// INVITE: Inviter un utilisateur à rejoindre le canal
void handleInviteCommand(ChannelManager& channelManager, MockServer& server, const std::string& channelName, const std::string& user, const std::string& targetUser) 
{
    try 
    {
        Channel& channel = channelManager.getChannel(channelName);
        if (channel.isOperator(user)) 
        {
            channel.addUser(targetUser);
            server.sendMessage(targetUser, "Vous avez été invité à rejoindre le canal " + channelName + " par " + user);
            std::cout << targetUser << " a été invité à rejoindre le canal " << channelName << " par " << user << std::endl;
        } 
        else 
        {
            std::cerr << "Erreur : " << user << " n'a pas les permissions nécessaires." << std::endl;
        }
    } 
    catch (const std::runtime_error& e) 
    {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }
}

// TOPIC: Modifier le sujet du canal
void handleTopicCommand(ChannelManager& channelManager, MockServer& server, const std::string& channelName, const std::string& user, const std::string& newTopic) 
{
    try 
    {
        Channel& channel = channelManager.getChannel(channelName);
        if (channel.isOperator(user)) 
        {
            channel.setTopic(newTopic);
            server.sendMessage(user, "Le sujet du canal " + channelName + " a été modifié par " + user);
            std::cout << "Le sujet du canal " << channelName << " a été modifié par " << user << std::endl;
        } 
        else 
        {
            std::cerr << "Erreur : " << user << " n'a pas les permissions nécessaires." << std::endl;
        }
    } 
    catch (const std::runtime_error& e) 
    {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }
}

// MODE: Modifier le mode du canal
void handleModeCommand(ChannelManager& channelManager, MockServer& server, const std::string& channelName, const std::string& user, const std::string& mode) 
{
    try 
    {
        Channel& channel = channelManager.getChannel(channelName);
        if (channel.isOperator(user)) 
        {
            if (mode == "+o") 
            {
                channel.addOperator(user);
            } 
            else if (mode == "-o") 
            {
                channel.removeOperator(user);
            }
            server.sendMessage(user, "Le mode du canal " + channelName + " a été modifié par " + user);
            std::cout << "Le mode du canal " << channelName << " a été modifié par " << user << std::endl;
        } 
        else 
        {
            std::cerr << "Erreur : " << user << " n'a pas les permissions nécessaires." << std::endl;
        }
    } 
    catch (const std::runtime_error& e) 
    {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }
}