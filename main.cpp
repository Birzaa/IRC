#include <iostream>
#include "ChannelManager.hpp"

int main()
{
    ChannelManager manager;

    // Créer un canal
    manager.createChannel("#general");

    // Ajouter un utilisateur et un opérateur
    Channel& generalChannel = manager.getChannel("#general");
    generalChannel.addUser("Alice");
    generalChannel.addOperator("Alice");

    // Vérifier si Alice est un opérateur
    if (generalChannel.isOperator("Alice"))
    {
        std::cout << "Alice est un opérateur du canal #general." << std::endl;
    }

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