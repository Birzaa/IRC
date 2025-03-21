#include "Commands.hpp"
#include "ChannelManager.hpp"
#include "MockServer.hpp"
#include <iostream>

int main() {
    ChannelManager channelManager;
    MockServer mockServer; // Ajoute une instance de MockServer

    // Créer un canal de test
    channelManager.createChannel("#general");

    // Tester la commande JOIN
    handleJoinCommand(channelManager, mockServer, "#general", "Alice"); // Passe mockServer en argument
    handleJoinCommand(channelManager, mockServer, "#general", "Bob");   // Passe mockServer en argument

    // Tester la commande PART
    handlePartCommand(channelManager, mockServer, "#general", "Alice"); // Passe mockServer en argument

    // Tester la commande KICK
    handleKickCommand(channelManager, mockServer, "#general", "Bob", "Alice"); // Passe mockServer en argument

    // Tester la commande INVITE
    handleInviteCommand(channelManager, mockServer, "#general", "Bob", "Charlie"); // Passe mockServer en argument

    // Tester la commande TOPIC
    handleTopicCommand(channelManager, mockServer, "#general", "Bob", "Bienvenue sur #general !"); // Passe mockServer en argument

    // Tester la commande MODE
    handleModeCommand(channelManager, mockServer, "#general", "Bob", "+o"); // Passe mockServer en argument

    std::cout << std::endl;

        // Test : Un utilisateur essaie de quitter un canal où il n'est pas présent
    handlePartCommand(channelManager, mockServer, "#general", "Eve"); // Eve n'est pas dans le canal

    // Test : Un utilisateur essaie d'exécuter une commande avancée sans être opérateur
    handleKickCommand(channelManager, mockServer, "#general", "Charlie", "Bob"); // Charlie n'est pas opérateur

    // Test : Un canal vide après le départ de tous les utilisateurs
    handlePartCommand(channelManager, mockServer, "#general", "Bob");
    handlePartCommand(channelManager, mockServer, "#general", "Charlie");

    return 0;
}