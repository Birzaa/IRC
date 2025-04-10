#include "Server.hpp"
#include "Utils.hpp"

const std::string Server::VALID_COMMANDS[12] = 
{
    "PASS", "NICK", "USER", "JOIN", "PART", "PRIVMSG",
    "MODE", "TOPIC", "INVITE", "KICK", "QUIT", "PING"
};

Server::Server(const int &port, const std::string &password) : _password(password)
{
    if (!isValidIRCPort(port))
        throw std::runtime_error(RED "Invalid port number" RESET);
    this->_port = port;
    this->_serverHost = "localhost";
}

Server::~Server()
{
    closeFds();
	for (size_t i = 0; i < _clients.size(); i++) 
    	delete _clients[i]; 

	std::cout << BOLD_RED << "\nServer closed" << RESET << std::endl;
}

void Server::initServer()
{
	initSocket();
	startServer();
}

// void Server::initSocket() {
//     // 1. Création du socket
//     _serverFd = socket(AF_INET, SOCK_STREAM, 0);
//     if (_serverFd == -1) {
//         throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));
//     }

//     // 2. Configuration des options (macOS compatible)
//     int opt = 1;
//     if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
//         close(_serverFd);
//         throw std::runtime_error("Setsockopt failed: " + std::string(strerror(errno)));
//     }

//     // 3. Configuration de l'adresse
//     struct sockaddr_in serverAddr;
//     memset(&serverAddr, 0, sizeof(serverAddr)); // Initialisation propre
//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_addr.s_addr = INADDR_ANY;
//     serverAddr.sin_port = htons(_port);

//     // 4. Bind
//     if (bind(_serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
//         close(_serverFd);
//         throw std::runtime_error("Bind failed: " + std::string(strerror(errno)));
//     }

//     // 5. Listen
//     if (listen(_serverFd, SOMAXCONN) < 0) {
//         close(_serverFd);
//         throw std::runtime_error("Listen failed: " + std::string(strerror(errno)));
//     }

//     // 6. Mode non-bloquant (comme dans ta version originale)
//     if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) < 0) {
//         close(_serverFd);
//         throw std::runtime_error("Fcntl failed: " + std::string(strerror(errno)));
//     }

//     // 7. Configuration de poll (comme dans ta version originale)
//     struct pollfd pfd;
//     pfd.fd = _serverFd;
//     pfd.events = POLLIN;
//     pfd.revents = 0;
//     _fds.push_back(pfd);

//     std::cout << GREEN << "Server started on port " << _port << RESET << std::endl;
// }

void Server::closeFds()
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		std::cout << printTime() << "Client : FD[" << this->_clients[i]->getFd() << "] Disconnected" << std::endl;
		close(this->_clients[i]->getFd());
	}
	if (this->_serverFd >= 0)
		close(this->_serverFd);
}

void Server::initSocket()
{
    // Create a socket
    this->_serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_serverFd == -1)
        throw std::runtime_error("Socket creation failed");

    // Set the socket options
    int opt = 1;
    if (setsockopt(this->_serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(int)) == -1) 
    {
        throw std::runtime_error("setsockopt SO_REUSEADDR failed");
    }
    
    
    //KEEPALIVE -> PING PONG to check if client is still connected
	if (setsockopt(this->_serverFd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) == -1) 
    {
        throw std::runtime_error("setsockopt SO_KEEPALIVE failed");
    }


    // set server configuration
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; // use IPv4
    serverAddr.sin_addr.s_addr = INADDR_ANY; // accept all connexions
    serverAddr.sin_port = htons(this->_port); //convert port to network language

    // Bind the socket to the address and port
    if (bind(this->_serverFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
        throw std::runtime_error("Bind failed");

    // Listen for incoming connexions
    if (listen(this->_serverFd, SOMAXCONN) == -1)
        throw std::runtime_error("Listen failed");

    // modifing the socket to non-blocking 
    if(fcntl(this->_serverFd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("Fcntl failed");

    // add server to poll()
    struct pollfd pfd;
    pfd.fd = this->_serverFd; // add the server socket to the poll watch list
    pfd.events = POLLIN; // check if there is data to read (ex : new client)
    this->_fds.push_back(pfd); // store the server socket in the poll list

    std::cout << GREEN << "Server started on port " << this->_port << RESET << std::endl;
}

void Server::startServer()
{
	std::cout << "Waiting for clients..." << std::endl;
    time_t lastPingCheck = time(NULL);

	while (Server::signal == false) 
	{
        if (time(NULL) - lastPingCheck > 10) 
        {
            checkPingTimeout();
            lastPingCheck = time(NULL);
        }
        
        // poll() surveille tous les FDs (_fds.data() pointe sur le 1er élement du vecteur), (-1) pour attendre indéfiniment
        int ret = poll(_fds.data(), _fds.size(), -1);
        if (ret == -1 && Server::signal == false) 
            throw std::runtime_error("Poll failed");

		// Parcours la liste des descripteurs de fichiers pour voir qui a déclenché un événement
        for (size_t i = 0; i < _fds.size(); i++) 
		{
			// Si le FD a déclenché un événement POLLIN (données à lire)
            if (_fds[i].revents & POLLIN) 
			{
                if (_fds[i].fd == _serverFd) 
                    acceptClient();  // Un nouveau client essaie de se connecter
                else
                    handleMessage(_fds[i].fd);  // Un client a envoyé un message
            }
        }
    }
	closeFds();
}

void Server::acceptClient() 
{
    Client* newClient = new Client();;
	struct sockaddr_in clientAddr;
	struct pollfd pfd;
	socklen_t len = sizeof(clientAddr);
	
	// Accepter la connexion du client
	int clientFd = accept(_serverFd, (struct sockaddr *)&clientAddr, &len);
	if (clientFd == -1) 
		throw std::runtime_error("Accept failed");

	// Mettre le client en mode non-bloquant
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1) 
		throw std::runtime_error("Fcntl failed");

	// Ajouter le client à la liste des clients
	newClient->setFd(clientFd);
	newClient->setIpClient(inet_ntoa(clientAddr.sin_addr)); // convert ip to string
	_clients.push_back(newClient); // add the client to the list of clients

	// Ajouter le client à la liste des FDs surveillés
	pfd.fd = clientFd; // on surveille le client
	pfd.events = POLLIN; // on surveille si le client envoie des données
	pfd.revents = 0; // on remet à 0 les événements
	_fds.push_back(pfd); // on ajoute le client à la liste des FDs surveillés

	std::cout << printTime() << "New connection FD [" << clientFd << "]" << std::endl;
}

void Server::handlePass(Client* client, std::istringstream& iss) 
{
    std::string password;
    iss >> password;

    // Nettoyer le mot de passe (retirer \r, \n, etc.)
    password.erase(std::remove(password.begin(), password.end(), '\r'), password.end());
    password.erase(std::remove(password.begin(), password.end(), '\n'), password.end());

    // Cas 1: Mot de passe vide
    if (password.empty()) 
    {
        std::string response = ":" + _serverHost + " 461 " + (client->getNickname().empty() ? "*" : client->getNickname()) + " PASS :Not enough parameters\r\n";
        send(client->getFd(), response.c_str(), response.size(), 0);
        return;
    }

    // Cas 2: Client déjà authentifié
    if (client->getAuthenticated()) 
    {
        std::string response = ":" + _serverHost + " 462 " + client->getNickname() + " :You may not reregister\r\n";
        send(client->getFd(), response.c_str(), response.size(), 0);
        return;
    }

    // Cas 3: Mot de passe correct
    if (password == _password) 
    {
        client->setAuthenticated(true);
        client->authAttempts = 3; // Réinitialiser les tentatives
    }

    // Cas 4: Mot de passe incorrect
    else 
    {
        client->authAttempts--;

        if (client->authAttempts <= 0) 
        {
            std::string errorMsg = ":" + _serverHost + " 464 " + (client->getNickname().empty() ? "*" : client->getNickname()) + " :Password incorrect - Too many attempts\r\n";
            send(client->getFd(), errorMsg.c_str(), errorMsg.size(), 0);
            removeClient(client->getFd());
        } 
        else 
        {
            std::string errorMsg = ":" + _serverHost + " 464 " + (client->getNickname().empty() ? "*" : client->getNickname()) + " :Password incorrect\r\n";
            send(client->getFd(), errorMsg.c_str(), errorMsg.size(), 0);
        }
    }
}



void Server::handleMessage(int clientFd) 
{
    char buffer[512];
    ssize_t bytes_received;
    static std::map<int, std::string> client_buffers;

    bytes_received = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received <= 0) 
    {
        if (bytes_received == 0) 
        {
            std::cout << printTime() << "Client disconnected (FD " << clientFd << ")" << std::endl;
        } 
        else 
        {
            std::cerr << printTime() << RED << " [ERROR] recv() failed for FD " << clientFd << RESET << std::endl;
        }

        Client* client = getClientByFd(clientFd);
        if (client) 
        {
            handleQuit(client);
        } 
        else 
        {
            removeClient(clientFd);
        }
        client_buffers.erase(clientFd);
        return;
    }

    buffer[bytes_received] = '\0';
    client_buffers[clientFd] += buffer;

    // Process complete messages (ending with \r\n)
    size_t end_pos;
    while ((end_pos = client_buffers[clientFd].find("\r\n")) != std::string::npos) 
    {
        std::string full_message = client_buffers[clientFd].substr(0, end_pos);
        client_buffers[clientFd].erase(0, end_pos + 2);

        // Clean the message
        full_message.erase(std::remove(full_message.begin(), full_message.end(), '\r'), full_message.end());
        full_message.erase(std::remove(full_message.begin(), full_message.end(), '\n'), full_message.end());
        full_message.erase(0, full_message.find_first_not_of(" \t"));
        full_message.erase(full_message.find_last_not_of(" \t") + 1);

        if (full_message.empty()) continue;

        Client* client = getClientByFd(clientFd);
        if (!client) 
        {
            std::cerr << RED << "Client not found for FD " << clientFd << RESET << std::endl;
            client_buffers.erase(clientFd);
            return;
        }
        
        // Log the message
        std::ostringstream oss;
        oss << clientFd;
        std::cout << printTime() << MAGENTA + (client->getNickname().empty() ? ("FD [" + oss.str() + "]: ") : client->getNickname()) + ": " RESET << full_message << std::endl;

        std::istringstream iss(full_message);
        std::string command;
        iss >> command;
        std::transform(command.begin(), command.end(), command.begin(), ::toupper);

        // Check if command is valid
        bool isValidCommand = false;
        for (size_t i = 0; i < sizeof(VALID_COMMANDS)/sizeof(VALID_COMMANDS[0]); ++i)
        {
            if (command == VALID_COMMANDS[i]) 
            {
                isValidCommand = true;
                break;
            }
        }

        // Handle CAP (ignore for now)
        if (command == "CAP") 
        {
            continue;
        }

        // Handle unknown commands
        if (!isValidCommand && (command != "PONG" && command != "CAP" && command != "CAP LS" && command != "WHO" && command != "WHOIS")) 
        {
            std::string errorMsg = ":" + _serverHost + " 421 " + (client->getNickname().empty() ? "*" : client->getNickname()) 
                                + " " + command + " :Unknown command\r\n";
            send(clientFd, errorMsg.c_str(), errorMsg.size(), 0);
            continue;
        }

        // Check authentication for non-PASS commands
        if (!client->getAuthenticated() && command != "PASS") 
        {
            std::string errorMsg = ":" + _serverHost + " 451 " + (client->getNickname().empty() ? "*" : client->getNickname()) 
                                + " :You must authenticate first\r\n";
            send(clientFd, errorMsg.c_str(), errorMsg.size(), 0);
            continue;
        }

        // Process the command
        try 
        {
            if (command == "PASS") handlePass(client, iss);
            else if (command == "NICK") handleNick(client, iss);
            else if (command == "USER") handleUser(client, iss);
            else if (command == "JOIN") createChannel(iss, client);
            else if (command == "PART") handlePart(client, iss);
            else if (command == "PRIVMSG") handlePrivmsg(client, iss);
            else if (command == "MODE") handleMode(client, iss);
            else if (command == "TOPIC") handleTopic(client, iss);
            else if (command == "INVITE") handleInvite(client, iss);
            else if (command == "KICK") handleKick(client, iss);
            else if (command == "QUIT") handleQuit(client);
            else if (command == "PING") handlePing(client, iss);
        } 
        catch (const std::exception& e) 
        {
            std::string errorMsg = ":" + _serverHost + " 500 " + (client->getNickname().empty() ? "*" : client->getNickname()) 
                                + " :Error processing command: " + std::string(e.what()) + "\r\n";
            send(clientFd, errorMsg.c_str(), errorMsg.size(), 0);
            std::cerr << RED << "Error processing command from FD " << clientFd 
                     << ": " << e.what() << RESET << std::endl;
        }
    }

    // Protection against buffer overflow (4KB max)
    if (client_buffers[clientFd].size() > 4096) 
    { 
        std::cerr << printTime() << RED << " Buffer overflow detected on FD " << clientFd << RESET << std::endl;
        removeClient(clientFd);
        client_buffers.erase(clientFd);
    }
}

void Server::removeClient(int clientFd) 
{
    if (clientFd < 0) return;

	if (clientFd >= 0)
   		close(clientFd);

    for (size_t i = 0; i < _clients.size(); i++) 
	{
        if (_clients[i]->getFd() == clientFd)
		{
			delete _clients[i];
            _clients.erase(_clients.begin() + i);  // Supprime l'élément au bon index
            break;
        }
    }

    // Retirer le client de la liste des FDs surveillés
    for (size_t i = 0; i < _fds.size(); i++) 
	{
        if (_fds[i].fd == clientFd) 
        {
            _fds.erase(_fds.begin() + i);
            break;
        }
    }

    std::cout << printTime() << "Client FD [" << clientFd << "] supprimé." << std::endl;
}

bool Server::signal = false;
void Server::signalHandler(int sig)
{
	(void)sig;
	// std::cout << " Signal " << sig << " received" << std::endl;
	Server::signal = true;
}

Client* Server::getClientByFd(int fd)
{
    for (size_t i = 0; i < _clients.size(); i++) 
    {
        if (_clients[i]->getFd() == fd) 
            return _clients[i];  // Retourne vers le client trouvé
    }
    return NULL;  // Si aucun client trouvé
}

void Server::handleNick(Client* client, std::istringstream& iss) 
{
    std::string nickname;
    iss >> nickname;

    // Vérification : Trop d'arguments après le nick
    std::string extra;
    if (iss >> extra) // Si on lit encore quelque chose, c'est une erreur
    {
        std::string response = ":" + _serverHost + " 461 ";
        response += (client->getNickname().empty() ? "*" : client->getNickname());
        response += " NICK :Too many parameters\r\n";
        send(client->getFd(), response.c_str(), response.size(), 0);
        return;
    }

    // Nettoyage du nickname
    std::string::iterator it;
    for (it = nickname.begin(); it != nickname.end(); ) 
    {
        if (*it == '\r' || *it == '\n') 
        {
            it = nickname.erase(it);
        } 
        else 
        {
            ++it;
        }
    }

    // Cas 1: Nickname vide (RFC 2812 section 4.1.2)
    if (nickname.empty()) 
    {
        std::string response = ":" + _serverHost + " 431 ";
        response += (client->getNickname().empty() ? "*" : client->getNickname());
        response += " :No nickname given\r\n";
        send(client->getFd(), response.c_str(), response.size(), 0);
        return;
    }

    // Validation de la longueur et premier caractère (RFC 2812 section 2.3.1)
    if (nickname.size() > 9 || !isalpha(nickname[0])) 
    {
        std::string response = ":" + _serverHost + " 432 ";
        response += (client->getNickname().empty() ? "*" : client->getNickname());
        response += " " + nickname + " :Erroneous nickname\r\n";
        send(client->getFd(), response.c_str(), response.size(), 0);
        return;
    }

    // Vérification des caractères valides
    const std::string special = "[]\\`_^{|}";
    bool valid = true;
    for (size_t i = 1; i < nickname.size(); ++i) 
    {
        if (!isalnum(nickname[i]) && nickname[i] != '-' && 
            special.find(nickname[i]) == std::string::npos) {
            valid = false;
            break;
        }
    }

    if (!valid) 
    {
        std::string response = ":" + _serverHost + " 432 ";
        response += (client->getNickname().empty() ? "*" : client->getNickname());
        response += " " + nickname + " :Erroneous nickname\r\n";
        send(client->getFd(), response.c_str(), response.size(), 0);
        return;
    }

    // Vérification si le nickname est déjà utilisé (RFC 2812 section 4.1.2)
    for (size_t i = 0; i < _clients.size(); i++) 
    {
        if (_clients[i]->getNickname() == nickname && _clients[i] != client) 
        {
            std::string response = ":" + _serverHost + " 433 ";
            response += (client->getNickname().empty() ? "*" : client->getNickname());
            response += " " + nickname + " :Nickname is already in use\r\n";
            send(client->getFd(), response.c_str(), response.size(), 0);
            return;
        }
    }



    // Notification de changement de nick
    std::string oldNick = client->getNickname();
    client->setNickname(nickname);
    


    if (!oldNick.empty()) 
    {
        std::string nickChangeMsg = ":" + oldNick + "!" + client->getUsername() + "@" + client->getHostname() + " NICK :" + nickname + "\r\n";
        
        // Envoyer à tous les channels où est présent le client
        std::map<std::string, Channel>::iterator chanIt;
        for (chanIt = _channels.begin(); chanIt != _channels.end(); ++chanIt) 
        {
            if (chanIt->second.isMember(client)) 
            {
                chanIt->second.broadcast2(nickChangeMsg, client);
            }
        }
        
        // Envoyer aussi au client lui-même
        send(client->getFd(), nickChangeMsg.c_str(), nickChangeMsg.size(), 0);
    } 
    else
        // Premier enregistrement du nick
        if (client->getAuthenticated() && !client->getUsername().empty())
            sendWelcomeMessage(client);
}

void Server::handleUser(Client* client, std::istringstream& iss) 
{
    std::string username, hostname, servername, realname;

    iss >> username >> hostname >> servername;
    std::getline(iss, realname);  // Capture le reste comme realname

     // Vérification du ':' pour realname (RFC 2812 Section 2.3.1)
     std::getline(iss, realname);
     size_t colon_pos = realname.find_first_of(":");
     if (colon_pos == std::string::npos) 
     {
        std::string response = ":" + _serverHost + " 461 Realname must start with ':'\r\n";\
        send(client->getFd(), response.c_str(), response.size(), 0);
         return;
     }
     realname = realname.substr(colon_pos + 1);
    
    // Suppression des espaces/tabulations en début/fin
    size_t start = realname.find_first_not_of(" \t");
    if (start != std::string::npos) 
    {
        realname = realname.substr(start);
    }
    size_t end = realname.find_last_not_of(" \t");
    if (end != std::string::npos) 
    {
        realname = realname.substr(0, end + 1);
    }

    // Vérification des paramètres
    if (username.empty() || hostname.empty() || servername.empty() || realname.empty()) 
    {
        std::string response = ":" + _serverHost + " 461 ";
        response += (client->getNickname().empty() ? "*" : client->getNickname());
        response += " USER :Not enough parameters\r\n";
        send(client->getFd(), response.c_str(), response.size(), 0);
        return;
    }

    // Vérification si déjà enregistré
    if (!client->getUsername().empty()) 
    {
        std::string response = ":" + _serverHost + " 462 ";
        response += (client->getNickname().empty() ? "*" : client->getNickname());
        response += " :You may not reregister\r\n";
        send(client->getFd(), response.c_str(), response.size(), 0);
        return;
    }

    // Enregistrement des informations utilisateur
    client->setUsername(username);
    client->setHostname(hostname);
    client->setServername(servername);
    client->setRealname(realname);

    // Envoi du message de bienvenue si le nick est déjà défini (RFC 2812 section 5.1)
    if (!client->getNickname().empty() && client->getAuthenticated())
       sendWelcomeMessage(client);
}

bool Server::isRegistered(Client *client)
{
	return !client->getNickname().empty() && !client->getUsername().empty() && !client->getHostname().empty() && !client->getServername().empty() && !client->getRealname().empty();
	// (void)client;
	// return 1;
}

void Server::handlePrivmsg(Client* client, std::istringstream& iss) 
{
    std::string target, message;

    // Vérification de l'enregistrement (RFC 2812 section 3.1)
    if (!isRegistered(client)) {
        std::string response = ":" + _serverHost + " 451 PRIVMSG :You have not registered\r\n";
        send(client->getFd(), response.c_str(), response.size(), 0);
        return;
    }

    // Extraction de la cible
    iss >> target;
    
    // Extraction du message
    std::getline(iss, message);
    
    // Nettoyage du message 
    size_t start = message.find_first_not_of(" \t");
    if (start != std::string::npos) 
    {
        message = message.substr(start);
    }
    
    // Validation des paramètres (RFC 2812 section 4.4.1)
    if (target.empty() || message.empty()) 
    {
        std::string response = ":" + _serverHost + " 461 " + client->getNickname() + " PRIVMSG :Not enough parameters\r\n";
        send(client->getFd(), response.c_str(), response.size(), 0);
        return;
    }

    if (message[0] != ':') 
    {
        std::string response = ":" + _serverHost + " 412 " + client->getNickname() + " :No text to send\r\n";
        send(client->getFd(), response.c_str(), response.size(), 0);
        return;
    }
    message = message.substr(1); // Supprimer le ':'

    // Construction du préfixe d'envoi
    std::string prefix = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname();

    // CAS 1: Message à un channel (commence par # ou &)
    if (target[0] == '#' || target[0] == '&') 
    {
        std::map<std::string, Channel>::iterator it = _channels.find(target);
        
        // Vérification de l'existence du channel (RFC 2812 section 4.4.1)
        if (it == _channels.end()) 
        {
            std::string response = ":" + _serverHost + " 403 " + client->getNickname() + " " + target + " :No such channel\r\n";
            send(client->getFd(), response.c_str(), response.size(), 0);
            return;
        }

        // Vérification de la présence dans le channel (RFC 2812 section 4.4.1)
        if (!it->second.isMember(client)) 
        {
            std::string response = ":" + _serverHost + " 404 " + client->getNickname() + " " + target + " :Cannot send to channel\r\n";
            send(client->getFd(), response.c_str(), response.size(), 0);
            return;
        }

        // Envoi du message au channel
        std::string fullMsg = prefix + " PRIVMSG " + target + " :" + message + "\r\n";
        it->second.broadcast2(fullMsg, client); // Ne renvoie pas à l'expéditeur
    }
    // CAS 2: Message privé
    else 
    {
        Client* recipient = NULL;
        for (size_t i = 0; i < _clients.size(); ++i) 
        {
            if (_clients[i]->getNickname() == target) 
            {
                recipient = _clients[i];
                break;
            }
        }

        // Vérification de l'existence du destinataire (RFC 2812 section 4.4.1)
        if (!recipient) 
        {
            std::string response = ":" + _serverHost + " 401 " + client->getNickname() + " " + target + " :No such nick/channel\r\n";
            send(client->getFd(), response.c_str(), response.size(), 0);
            return;
        }

        // Envoi du message privé
        std::string fullMsg = prefix + " PRIVMSG " + target + " :" + message + "\r\n";
        send(recipient->getFd(), fullMsg.c_str(), fullMsg.size(), 0);
    }
}



//                                Channel

void Server::createChannel(std::istringstream& iss, Client* client) 
{
    std::string channelName, password;
    iss >> channelName >> password;

    if (!isRegistered(client)) 
    {
        std::string response = ":" + _serverHost + " 451 JOIN :You have not registered\r\n";
        send(client->getFd(), response.c_str(), response.size(), 0);
        return;
    }

    channelName.erase(std::remove(channelName.begin(), channelName.end(), '\r'), channelName.end());
    channelName.erase(std::remove(channelName.begin(), channelName.end(), '\n'), channelName.end());
    password.erase(std::remove(password.begin(), password.end(), '\r'), password.end());
    password.erase(std::remove(password.begin(), password.end(), '\n'), password.end());

    // Validation basique
    if (channelName.empty() || (channelName[0] != '#' && channelName[0] != '&') || channelName.size() > 50 || channelName.size() < 2)
    {
        std::string msg = ":" + _serverHost + " 403 " + client->getNickname() + " "+ channelName +  " :Invalid channel name\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // Recherche ou création du channel
    std::map<std::string, Channel>::iterator it = _channels.find(channelName);
    if (it == _channels.end())
    {
        // Création d'un nouveau channel
        Channel newChannel(channelName);
        _channels.insert(std::make_pair(channelName, newChannel));
        it = _channels.find(channelName);
    }
    // Tentative de rejoindre
    it->second.addClient(client, password);
}

void Server::handlePart(Client* client, std::istringstream& iss) 
{
    std::string channelName;
    iss >> channelName;

    if (!isRegistered(client)) 
    {
        std::string response = ":" + _serverHost + " 451 PART :You have not registered\r\n";
        send(client->getFd(), response.c_str(), response.size(), 0);
        return;
    }

    // Nettoyage du nom de channel
    channelName.erase(std::remove(channelName.begin(), channelName.end(), '\r'), channelName.end());
    channelName.erase(std::remove(channelName.begin(), channelName.end(), '\n'), channelName.end());

    // Validation
    if (channelName.empty()) 
    {
        std::string reply = ":" + _serverHost + " 461 " + client->getNickname() + " PART :Not enough parameters\r\n";
        send(client->getFd(), reply.c_str(), reply.size(), 0);
        return;
    }

    // Vérification de l'existence du channel
    std::map<std::string, Channel>::iterator it = _channels.find(channelName);
    if (it == _channels.end()) 
    {
        std::string reply = ":" + _serverHost + " 403 " + client->getNickname() + " " + channelName + " :No such channel\r\n";
        send(client->getFd(), reply.c_str(), reply.size(), 0);
        return;
    }

    // Vérification de la présence dans le channel
    if (!it->second.isMember(client)) 
    {
        std::string reply = ":" + _serverHost + " 442 " + client->getNickname() + " " + channelName + " :You're not on that channel\r\n";
        send(client->getFd(), reply.c_str(), reply.size(), 0);
        return;
    }

    // Notification PART
    std::string partMsg = ":" + getClientHostmask(client) + " PART " + channelName + " :Leaving\r\n";
    
    // Envoyer à tous les membres du channel
    it->second.broadcast(partMsg);
    
    // Retirer le client du channel
    it->second.removeClient(client, " PART ");

    // Supprimer le channel si vide
    if (it->second.getClients().empty()) 
    {
        _channels.erase(it);
        std::cout << printTime() << "Channel " << channelName << " deleted (no more members)\n";
    }
}

std::string Server::getClientHostmask(Client* client) const 
{
    std::string hostmask;
    
    // Nickname (ou * si non défini)
    hostmask += client->getNickname().empty() ? "*" : client->getNickname();
    hostmask += "!";
    
    // Username (ou * si non défini)
    hostmask += client->getUsername().empty() ? "*" : client->getUsername();
    hostmask += "@";
    
    // Hostname/IP
    if (!client->getHostname().empty()) 
    {
        hostmask += client->getHostname();
    }
    else 
    {
        // Fallback sur l'IP si hostname non défini
        hostmask += client->getIpClient();
    }
    
    return hostmask;
}

void Server::handleQuit(Client* client) 
{
    // Construire le message QUIT conforme RFC 2812
    std::string quitMsg = ":" + getClientHostmask(client) + " QUIT";
    quitMsg += "\r\n";

    // Notifier tous les channels où le client est présent
    std::map<std::string, Channel>::iterator it = _channels.begin();
    while (it != _channels.end()) 
    {
        Channel& channel = it->second;
        
        if (channel.isMember(client)) 
        {
            // 1. Notifier les membres du channel
            channel.broadcast(quitMsg);
            
            // 2. Retirer le client du channel
            channel.removeClient(client, " QUIT ");
            
            // 3. Supprimer les channels vides
            if (channel.getClients().empty()) 
            {
                std::cout << printTime() << "Channel " << it->first 
                          << " deleted (last member quit)\n";
                _channels.erase(it++);
                continue;
            }
        }
        ++it;
    }

    // Fermer la connexion
    removeClient(client->getFd());
}

void Server::handleKick(Client* client, std::istringstream& iss) 
{
    std::string channelName, targetNick, reason;
    iss >> channelName >> targetNick;
    std::getline(iss, reason);

    if (!isRegistered(client)) 
    {
        std::string response = ":" + _serverHost + " 451 KICK :You have not registered\r\n";
        send(client->getFd(), response.c_str(), response.size(), 0);
        return;
    }

    // Nettoyage des paramètres
    channelName.erase(std::remove(channelName.begin(), channelName.end(), '\r'), channelName.end());
    channelName.erase(std::remove(channelName.begin(), channelName.end(), '\n'), channelName.end());
    targetNick.erase(std::remove(targetNick.begin(), targetNick.end(), '\r'), targetNick.end());
    targetNick.erase(std::remove(targetNick.begin(), targetNick.end(), '\n'), targetNick.end());
    
    // Nettoyage de la raison
    size_t start = reason.find_first_not_of(" \t");
    if (start != std::string::npos) 
    {
        reason = reason.substr(start);
    }
    if (!reason.empty() && reason[0] == ':') 
    {
        reason = reason.substr(1);
    }

    // Vérification des paramètres (RFC 2812 section 4.2.8)
    if (channelName.empty() || targetNick.empty()) 
    {
        std::string msg = ":" + _serverHost + " 461 " + client->getNickname() + " KICK :Not enough parameters\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // Vérification de l'existence du channel
    std::map<std::string, Channel>::iterator chanIt = _channels.find(channelName);
    if (chanIt == _channels.end()) 
    {
        std::string msg = ":" + _serverHost + " 403 " + channelName + " :No such channel\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }
    Channel& channel = chanIt->second;

    // Vérification des privilèges (RFC 2812 section 4.2.8)
    if (!channel.isOperator(client)) 
    {
        std::string msg = ":" + _serverHost + " 482 " + channelName + " :You're not channel operator\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // Recherche du client cible
    Client* target = channel.getClient(targetNick);
    if (!target) 
    {
        std::string msg = ":" + _serverHost + " 441 " + targetNick + " " + channelName + " :They aren't on that channel\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // Construction du message KICK
    std::string kickMsg = ":" + getClientHostmask(client) + " KICK " + channelName + " " + targetNick;
    if (!reason.empty()) 
    {
        kickMsg += " :" + reason;
    }
    kickMsg += "\r\n";

    // Broadcast du message KICK à tout le channel
    channel.broadcast(kickMsg);

    // Retrait du channel
    channel.removeClient(target, " KICKED ");

    // Journalisation
    std::cout << printTime() << "KICK: " << client->getNickname() 
              << " kicked " << targetNick << " from " << channelName 
              << " (" << (reason.empty() ? "no reason" : reason) << ")" << std::endl;
}

// Gestion de la commande INVITE
void Server::handleInvite(Client* client, std::istringstream& iss) 
{
    std::string targetNick, channelName;
    iss >> targetNick >> channelName;

    if (!isRegistered(client)) 
    {
        std::string response = ":" + _serverHost + " 451 KICK :You have not registered\r\n";
        send(client->getFd(), response.c_str(), response.size(), 0);
        return;
    }

    if (targetNick.empty() || channelName.empty()) 
    {
        std::string msg = ":" + _serverHost + " 461 " + client->getNickname() + " INVITE :Not enough parameters\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // Vérifier que le channel existe
    std::map<std::string, Channel>::iterator it = _channels.find(channelName);
    if (it == _channels.end()) 
    {
        std::string msg = ":" + _serverHost + " 403 " + channelName + " :No such channel\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // Vérifier que l'expéditeur est opérateur
    if (!it->second.isOperator(client) && _inviteOnly) 
    {
        std::string msg = ":" + _serverHost + " 482 " + channelName + " :You're not channel operator\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // Trouver le client cible
    Client* target = NULL;
    for (size_t i = 0; i < _clients.size(); i++) 
    {
        if (_clients[i]->getNickname() == targetNick) 
        {
            target = _clients[i];
            break;
        }
    }

    if (!target) 
    {
        std::string msg = ":" + _serverHost + " 401 " + targetNick + " :No such nick\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // Ajouter à la liste des invités
    it->second.addInvited(targetNick);

    // Envoyer les notifications
    // std::string msg = ":" + _serverHost + " 341 " + targetNick + " " + channelName + " :Inviting you to " + channelName + "\r\n";
    // send(target->getFd(), msg.c_str(), msg.size(), 0);

    // Notification à la cible
    std::string inviteMsg = ":" + getClientHostmask(client) + " INVITE " + targetNick + " " + channelName + "\r\n";
    send(target->getFd(), inviteMsg.c_str(), inviteMsg.size(), 0);
}

// Gestion de la commande TOPIC
void Server::handleTopic(Client* client, std::istringstream& iss) 
{
    std::string channelName, newTopic;
    iss >> channelName;
    std::getline(iss, newTopic);

    if (!isRegistered(client)) 
    {
        std::string response = ":" + _serverHost + " 451 TOPIC :You have not registered\r\n";
        send(client->getFd(), response.c_str(), response.size(), 0);
        return;
    }

    channelName.erase(std::remove(channelName.begin(), channelName.end(), '\r'), channelName.end());
    channelName.erase(std::remove(channelName.begin(), channelName.end(), '\n'), channelName.end());
    

    // Nettoyer le topic
    size_t start = newTopic.find_first_not_of(" \t");
    if (start != std::string::npos) 
    {
        newTopic = newTopic.substr(start);
    }
    if (!newTopic.empty() && newTopic[0] == ':') 
    {
        newTopic = newTopic.substr(1);
    }

    if (channelName.empty()) 
    {
        std::string msg = ":" + _serverHost + " 461 " + client->getNickname() + " TOPIC :Not enough parameters\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }
    // Vérifier que le channel existe
    std::map<std::string, Channel>::iterator it = _channels.find(channelName);
    if (it == _channels.end()) 
    {
        std::string msg = ":" + _serverHost + " 403 " + channelName + " :No such channel\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }
    Channel& channel = it->second;
    // Si pas de nouveau topic, afficher le topic actuel
    if (newTopic.empty()) 
    {
        std::string currentTopic = it->second.getTopic();
        if (currentTopic.empty()) 
        {
            std::string msg = ":" + _serverHost + " 331 " + channelName + " :No topic is set\r\n";
            send(client->getFd(), msg.c_str(), msg.size(), 0);
        } 
        else 
        {
                std::string msg = ":" + _serverHost + " 332 " + channelName + " :" + currentTopic + "\r\n";
                send(client->getFd(), msg.c_str(), msg.size(), 0);
        }
        return;
    }

    // Vérifier les permissions pour changer le topic
    if (it->second.getTopicRestricted() && !it->second.isOperator(client)) 
    {
        std::string msg = ":" + _serverHost + " 482 " + channelName + " :You're not channel operator\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // Changer le topic
    it->second.setTopic(newTopic);

    // Notifier tous les membres du channel
    std::string topicMsg = ":" + getClientHostmask(client) + " TOPIC " + channelName + " :" + newTopic + "\r\n";
    channel.broadcast(topicMsg);
}

// Gestion de la commande MODE
void Server::handleMode(Client* client, std::istringstream& iss) 
{
    std::string target, mode, arg;
    iss >> target >> mode;

    if (target == client->getNickname() && mode == "+i") 
    {
        return;
    }

    if (!isRegistered(client)) 
    {
        std::string response = ":" + _serverHost + " 451 MODE :You have not registered\r\n";
        send(client->getFd(), response.c_str(), response.size(), 0);
        return;
    }

    // Vérifier que la cible est un channel
    if (target.empty() || (target[0] != '#' && target[0] != '&')) 
    {
        std::string reply = ":" + _serverHost + " 472 " + client->getNickname() + " :Cannot change mode for non-channels\r\n";
        send(client->getFd(), reply.c_str(), reply.size(), 0);
        return;
    }

    // Vérifier que le channel existe
    std::map<std::string, Channel>::iterator it = _channels.find(target);
    if (it == _channels.end()) 
    {
        std::string reply = ":" + _serverHost + " 403 " + client->getNickname() + " " + target + " :No such channel\r\n";
        send(client->getFd(), reply.c_str(), reply.size(), 0);
        return;
    }

    // Vérifier que l'expéditeur est opérateur
    if (!it->second.isOperator(client)) 
    {
        std::string reply = ":" + _serverHost + " 482 " + client->getNickname() + " " + target + " :You're not channel operator\r\n";
        send(client->getFd(), reply.c_str(), reply.size(), 0);
        return;
    }

    // Si pas de mode, afficher les modes actuels
    if (mode.empty()) 
    {
        std::string modes = "+";
        if (it->second.getInviteOnly()) modes += "i";
        if (it->second.getTopicRestricted()) modes += "t";
        if (!it->second.getPassword().empty()) modes += "k";
        if (it->second.getMaxClients() > 0) modes += "l";
        
        std::string reply = ":" + _serverHost + " 324 " + client->getNickname() + " " + target + " " + modes + "\r\n";
        reply += ":" + _serverHost + " 329 " + client->getNickname() + " " + target + "\r\n";
        send(client->getFd(), reply.c_str(), reply.size(), 0);
        return;
    }

    // Validation du format du mode
    if (mode.size() != 2 || (mode[0] != '+' && mode[0] != '-')) 
    {
        std::string reply = ":" + _serverHost + " 501 " + client->getNickname() + " :Unknown MODE flag\r\n";
        send(client->getFd(), reply.c_str(), reply.size(), 0);
        return;
    }


    // Traiter les modes
    bool addMode = (mode[0] == '+');
    char modeChar = mode[1];
    iss >> arg;

    switch (modeChar) 
    {
        case 'i': // Mode invitation seulement
            it->second.setInviteOnly(addMode);
            break;
        case 't': // Restriction du topic aux ops
            it->second.setTopicRestricted(addMode);
            break;
        case 'k': // Mot de passe du channel
    if (addMode) 
    {
        if (arg.empty()) 
        {
                    std::string reply = ":" + _serverHost + " 461 " + client->getNickname() + " MODE :Not enough parameters\r\n";
                    send(client->getFd(), reply.c_str(), reply.size(), 0);
                    return;
        }
        it->second.setPassword(arg);
    }
    else 
    {
        it->second.setPassword("");
        // Envoyer une notification que le mot de passe est supprimé
        std::string msg = ":" + client->getNickname() + " MODE " + target + " -k\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
    }
    break;
        case 'o': // Donner/retirer le statut opérateur
            {
                if (arg.empty()) 
                {
                    std::string reply = ":" + _serverHost + " 461 " + client->getNickname() + " MODE :Not enough parameters\r\n";
                    send(client->getFd(), reply.c_str(), reply.size(), 0);
                    return;
                }
                Client* targetClient = NULL;
                const std::vector<Client*>& members = it->second.getClients();
                for (size_t i = 0; i < members.size(); ++i) 
                {
                    if (members[i]->getNickname() == arg) 
                    {
                        targetClient = members[i];
                        break;
                    }
                }
                if (!targetClient) 
                {
                    std::string reply = ":" + _serverHost + " 441 " + client->getNickname() + " " + arg + " " + target + " :They aren't on that channel\r\n";
                    send(client->getFd(), reply.c_str(), reply.size(), 0);
                    return;
                }
                if (addMode) 
                {
                    it->second.addOperator(targetClient);
                } 
                else 
                {
                    it->second.removeOperator(targetClient);
                }
            }
            break;
        case 'l': // Limite d'utilisateurs
            if (addMode) 
            {
                if (arg.empty()) 
                {
                    std::string reply = ":" + _serverHost + " 461 " + client->getNickname() + " MODE :Not enough parameters\r\n";
                    send(client->getFd(), reply.c_str(), reply.size(), 0);
                    return;
                }
                int limit = atoi(arg.c_str());
                if (limit <= 0) 
                {
                    std::string reply = ":" + _serverHost + " 696 " + client->getNickname() + " " + target + " " + mode + " :Invalid limit value\r\n";
                    send(client->getFd(), reply.c_str(), reply.size(), 0);
                    return;
                }
                it->second.setMaxClients(limit);
            } 
            else 
            {
                it->second.setMaxClients(-1);
            }
            break;
        default:
            std::string reply = ":" + _serverHost + " 472 " + client->getNickname() + " " + std::string(1, modeChar) + " :Unknown MODE flag\r\n";
            send(client->getFd(), reply.c_str(), reply.size(), 0);
            return;
    }

    // Notifier le changement de mode
    std::string modeMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + " MODE " + target + " " + mode;
    if (!arg.empty()) modeMsg += " " + arg;
    modeMsg += "\r\n";
    
    const std::vector<Client*>& members = it->second.getClients();
    for (size_t i = 0; i < members.size(); ++i) 
    {
        send(members[i]->getFd(), modeMsg.c_str(), modeMsg.size(), 0);
    }
}

void Server::sendWelcomeMessage(Client* client) 
{
    // std::string welcomeMsg = ":" + _serverHost + " 001 " + client->getNickname() + 
    //                        " :Welcome to the Internet Relay Network " + 
    //                        client->getNickname() + "!" + client->getUsername() + 
    //                        "@" + client->getHostname() + "\r\n";
    
    std::string welcomeMsg = ":" + _serverHost + " 001 " + client->getNickname() + 
                           " :Welcome to the Internet Relay Network " + \
                           client->getNickname() + "\r\n";
    
    send(client->getFd(), welcomeMsg.c_str(), welcomeMsg.size(), 0);
    
}



void Server::handlePing(Client* client, std::istringstream& iss) 
{
    std::string token;
    std::getline(iss, token);
    
    // Nettoyer le token
    token.erase(std::remove(token.begin(), token.end(), '\r'), token.end());
    token.erase(std::remove(token.begin(), token.end(), '\n'), token.end());
    
    // Supprimer les espaces et le ':' au début si présent
    size_t start = token.find_first_not_of(" :");
    if (start != std::string::npos) 
    {
        token = token.substr(start);
    }

    // Envoyer la réponse PONG
    std::string response = "PONG :" + token + "\r\n";
    send(client->getFd(), response.c_str(), response.size(), 0);
    
    // Mettre à jour le dernier PONG reçu (pour le timeout)
    client->setLastPong(time(NULL));
}

void Server::checkPingTimeout() 
{
    time_t now = time(NULL);
    
    for (size_t i = 0; i < _clients.size(); ++i) 
    {
        Client* client = _clients[i];
        time_t lastActive = client->getLastPong();
        
        if (now - lastActive > PING_TIMEOUT) 
        {
            std::cout << printTime() << "Client " << client->getNickname() << " timed out (no PONG response)" << std::endl;
            handleQuit(client);
            --i; // Ajuster l'index après suppression
        } 
        else if (now - lastActive > PING_INTERVAL) 
        {
            // Envoyer un PING si le client n'a pas répondu récemment
            std::string pingMsg = "PING :" + _serverHost + "\r\n";
            send(client->getFd(), pingMsg.c_str(), pingMsg.size(), 0);
        }
    }
}