#include "Server.hpp"
#include <csignal>
#include <cstdlib> 

int main(int ac, char **av) 
{
    if (ac != 3) 
    {
        std::cerr << RED << "Error: " << "./irc <port> <password>" << RESET << std::endl;
        return EXIT_FAILURE;
    }

    try 
    {
        // 1. Validation du port
        int port = checkAtoi(av[1]);
        
        // 2. Création du serveur
        Server server(port, av[2]);
        
        // 3. Configuration des signaux
        signal(SIGINT, Server::signalHandler);  // CTRL+C
        signal(SIGQUIT, Server::signalHandler); // (CTRL+\)
        signal(SIGPIPE, SIG_IGN); // Ignorer SIGPIPE
        
        // 4. Initialisation et démarrage
        server.initServer();
        
    } 
    catch (const std::exception& e) 
    {
        // Toutes les exceptions sont attrapées ici
        std::cerr << RED << "Fatal error: " << e.what() << RESET << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}