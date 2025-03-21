#include <iostream>
#include "ChannelManager.hpp"
#include "Commands.hpp"

int main()
{
    ChannelManager manager;

    // Créer un canal
    manager.createChannel("#general");

    // Ajouter un utilisateur et un opérateur
    handleJoinCommand(manager, "#general", "Alice");
    handleJoinCommand(manager, "#general", "Bob");

    // Donner les permissions d'opérateur à Alice
    handleModeCommand(manager, "#general", "Alice", "+o");

    // Vérifier si Alice est un opérateur
    Channel& generalChannel = manager.getChannel("#general");
    if (generalChannel.isOperator("Alice"))
    {
        std::cout << "Alice est un opérateur du canal #general." << std::endl;
    }

    // Tester la commande KICK (Alice expulse Bob)
    handleKickCommand(manager, "#general", "Alice", "Bob");

    // Tester la commande INVITE (Alice invite Charlie)
    handleInviteCommand(manager, "#general", "Alice", "Charlie");

    // Tester la commande TOPIC (Alice modifie le sujet)
    handleTopicCommand(manager, "#general", "Alice", "Bienvenue sur #general !");

    // Tester la commande MODE (Alice donne les permissions d'opérateur à Bob)
    handleModeCommand(manager, "#general", "Alice", "+o Bob");

    // Supprimer un canal
    manager.deleteChannel("#general");

    // Essayer d'accéder à un canal supprimé (lève une exception)
    try
    {
        manager.getChannel("#general"); // Pas besoin de stocker dans une variable
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }

    return 0;
}