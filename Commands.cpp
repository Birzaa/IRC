#include "Commands.hpp"
#include <iostream> // Pour std::cout et std::endl
#include <stdexcept> // Pour std::runtime_error

void handleJoinCommand(ChannelManager& channelManager, const std::string& channelName, const std::string& user)
{
    try {
        Channel& channel = channelManager.getChannel(channelName);
        channel.addUser(user);
        std::cout << user << " a rejoint le canal " << channelName << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }
}

void handlePartCommand(ChannelManager& channelManager, const std::string& channelName, const std::string& user)
{
    try {
        Channel& channel = channelManager.getChannel(channelName);
        channel.removeUser(user);
        std::cout << user << " a quitté le canal " << channelName << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }
}

void handleKickCommand(ChannelManager& channelManager, const std::string& channelName, const std::string& user, const std::string& targetUser)
{
    try {
        Channel& channel = channelManager.getChannel(channelName);
        if (channel.isOperator(user)) {
            channel.removeUser(targetUser);
            std::cout << targetUser << " a été expulsé du canal " << channelName << " par " << user << std::endl;
        } else {
            std::cerr << "Erreur : " << user << " n'a pas les permissions nécessaires." << std::endl;
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }
}

void handleInviteCommand(ChannelManager& channelManager, const std::string& channelName, const std::string& user, const std::string& targetUser)
{
    try {
        Channel& channel = channelManager.getChannel(channelName);
        if (channel.isOperator(user)) {
            channel.addUser(targetUser);
            std::cout << targetUser << " a été invité à rejoindre le canal " << channelName << " par " << user << std::endl;
        } else {
            std::cerr << "Erreur : " << user << " n'a pas les permissions nécessaires." << std::endl;
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }
}

void handleTopicCommand(ChannelManager& channelManager, const std::string& channelName, const std::string& user, const std::string& newTopic)
{
    try {
        Channel& channel = channelManager.getChannel(channelName);
        if (channel.isOperator(user)) {
            channel.setTopic(newTopic);
            std::cout << "Le sujet du canal " << channelName << " a été modifié par " << user << std::endl;
        } else {
            std::cerr << "Erreur : " << user << " n'a pas les permissions nécessaires." << std::endl;
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }
}

void handleModeCommand(ChannelManager& channelManager, const std::string& channelName, const std::string& user, const std::string& mode)
{
    try {
        Channel& channel = channelManager.getChannel(channelName);
        if (channel.isOperator(user)) {
            if (mode == "+o") {
                channel.addOperator(user);
            } else if (mode == "-o") {
                channel.removeOperator(user);
            }
            std::cout << "Le mode du canal " << channelName << " a été modifié par " << user << std::endl;
        } else {
            std::cerr << "Erreur : " << user << " n'a pas les permissions nécessaires." << std::endl;
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }
}

