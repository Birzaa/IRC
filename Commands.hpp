#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "Server.hpp"
#include <vector>
#include <string>

// Fonctions de commandes
void handleJoin(Server* server, Client* client, std::istringstream& iss);
void handlePart(Server* server, Client* client, std::istringstream& iss);
void handleKick(Server* server, Client* client, std::istringstream& iss);
void handleInvite(Server* server, Client* client, std::istringstream& iss);
void handleTopic(Server* server, Client* client, std::istringstream& iss);
void handleMode(Server* server, Client* client, std::istringstream& iss);

#endif