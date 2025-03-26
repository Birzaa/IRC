#include "Server.hpp"

Server::Server(const std::string &port, const std::string &password)
{
	this->_port = atoi(port.c_str());
	this->_password = password;
}

Server::~Server()
{
	std::cout << BOLD_RED << "\nServer closed" << RESET << std::endl;
}

void Server::initServer()
{
	initSocket();
	startServer();
}

void Server::initSocket()
{
	// Create a socket
	this->_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_serverFd == -1)
		throw std::runtime_error("Socket creation failed");
	
	// Set the socket options
	int opt = 1;
	if (setsockopt(this->_serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
		throw std::runtime_error("Setsockopt failed");
	
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

void Server::closeFds()
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		std::cout << ORANGE << "Client : " << this->_clients[i].getFd() << " Disconnected" << RESET << std::endl;
		close(this->_clients[i].getFd());
	}
	if (this->_serverFd >= 0)
		close(this->_serverFd);
}

void Server::startServer()
{
	std::cout << "Waiting for clients..." << std::endl;

	while (Server::signal == false) 
	{
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
    Client newClient;
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
	newClient.setFd(clientFd);
	newClient.setIpClient(inet_ntoa(clientAddr.sin_addr)); // convert ip to string
	_clients.push_back(newClient); // add the client to the list of clients

	// Ajouter le client à la liste des FDs surveillés
	pfd.fd = clientFd; // on surveille le client
	pfd.events = POLLIN; // on surveille si le client envoie des données
	pfd.revents = 0; // on remet à 0 les événements
	_fds.push_back(pfd); // on ajoute le client à la liste des FDs surveillés

	std::cout << BLUE <<"New client connected : FD " << clientFd << RESET << std::endl;
}

void Server::handleMessage(int clientFd) 
{
    char buffer[512];

	// Recevoir le message du client et le stocker dans le buffer
    ssize_t bytes_received = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received <= 0) 
	{
        std::cout << ORANGE << "Client déconnecté: FD " << clientFd << RESET << std::endl;
        removeClient(clientFd);
        return;
    }

    buffer[bytes_received] = '\0';
    std::string message(buffer); // convert buffer to string
	message.erase(message.find_last_not_of(" \n\r\t") + 1);
	if (message.empty())
        return;

	Client *client = getClientByFd(clientFd);
	if (!client)
		throw std::runtime_error("Client not found");
	
	// Vérification du mot de passe
	if (!client->getAuthenticated())
	{
		if (message.substr(0, 5) == "PASS ")
		{
			std::string pass = message.substr(5);
			pass.erase(std::remove(pass.begin(), pass.end(), '\n'), pass.end());
			if (pass == this->_password)
			{
				client->setAuthenticated(true);
				client->authAttempts = 3;
				send(clientFd, MAGENTA "~> Password accepted\n" RESET, 33, 0);
			}
			else
			{
				client->authAttempts--;
				if (client->authAttempts == 0)
				{
					send(clientFd, BOLD_RED"~> Too many authentication attempts\n" RESET, 48, 0);
					removeClient(clientFd);
				}
				else
					send(clientFd, CYAN "~> Password refused\n" RESET, 32, 0);
			}
		}
		else
			send(clientFd, CYAN "~> Please authenticate with PASS <password>\n" RESET, 56, 0);
	}

	
	// Gerer les messages quand on est connecté
	if (client->getAuthenticated()) 
	{
		std::istringstream iss(message);
		std::string command;
		iss >> command;

		if (command == "NICK")
			handleNick(client, iss);
		else if (command == "USER")
			handleUser(client, iss);
		else if (command == "PRIVMSG")
			handlePrivmsg(client, iss);
	}

}

void Server::removeClient(int clientFd) 
{
	if (clientFd >= 0)
   		close(clientFd);

    // Rechercher le client dans le vecteur (par exemple, par clientFd)
    for (size_t i = 0; i < _clients.size(); i++) 
	{
        if (_clients[i].getFd() == clientFd) {  // Supposons qu'il y ait une méthode `getFd()` dans `Client`
            _clients.erase(_clients.begin() + i);  // Supprime l'élément au bon index
            break;
        }
    }

    // Retirer le client de la liste des FDs surveillés
    for (size_t i = 0; i < _fds.size(); i++) 
	{
        if (_fds[i].fd == clientFd) {
            _fds.erase(_fds.begin() + i);
            break;
        }
    }

    std::cout << RED << "Client FD " << clientFd << " supprimé." << RESET << std::endl;
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
        if (_clients[i].getFd() == fd) 
            return &_clients[i];  // Retourne un pointeur vers le client trouvé
    }
    return NULL;  // Si aucun client trouvé
}

void Server::handleNick(Client *client, std::istringstream &iss)
{
    std::string nickname;
    iss >> nickname;

    if (nickname.empty())
    {
        send(client->getFd(), CYAN "~> Usage: NICK <nickname>\n" RESET, 38, 0);
        return;
    }
	 // Vérifier si le nickname est déjà pris par un autre client
    for (size_t i = 0; i < _clients.size(); i++) 
    {
        if (_clients[i].getNickname() == nickname) 
        {
            send(client->getFd(), CYAN"~> Nickname is already taken\n" RESET, 41, 0);
            return;
        }
    }

    client->setNickname(nickname);
    send(client->getFd(), MAGENTA "~> Nickname set successfully\n" RESET, 41, 0);
}

void Server::handleUser(Client* client, std::istringstream &iss) {
    std::string username, hostname, servername, realname;

    iss >> username >> hostname >> servername;
    std::getline(iss, realname);  // Capturer le reste comme le vrai nom (realname)

    // Vérifier si les informations sont valides
    if (username.empty() || hostname.empty() || servername.empty() || realname.empty()) {
        send(client->getFd(), CYAN "~> Usage: USER <username> <hostname> <servername> <realname>\n" RESET, 73, 0);
        return;
    }

    if (client->getUsername().empty()) 
	{
        client->setUsername(username);
        client->setHostname(hostname);
        client->setServername(servername);
        client->setRealname(realname);

        send(client->getFd(), MAGENTA "~> User information set successfully\n" RESET, 49, 0);
    } else
        send(client->getFd(), CYAN "~> You have already set your user information\n" RESET, 58, 0);
}

bool Server::isRegistered(Client *client)
{
	return !client->getNickname().empty() && !client->getUsername().empty() && !client->getHostname().empty() && !client->getServername().empty() && !client->getRealname().empty();
}

void Server::handlePrivmsg(Client *client, std::istringstream &iss)
{
    std::string nickname, message;

    if (!isRegistered(client))
    {
        send(client->getFd(), CYAN "~> You must set your nickname and user information first\n" RESET, 69, 0);
        return;
    }

    // Extraire le nickname du destinataire
    iss >> nickname;
    
    // Vérification si le message est vide
    std::getline(iss, message);
    if (message.empty())
    {
        send(client->getFd(), CYAN "~> Message cannot be empty\n" RESET, 39, 0);
        return;
    }

    // Supprimer les espaces au début du message
    message.erase(0, message.find_first_not_of(' '));

    // Vérifier si le message commence par un ':'
    if (message.empty() || message[0] != ':')
    {
        send(client->getFd(), CYAN "~> Usage: PRIVMSG <nickname> :<message>\n" RESET, 52, 0);
        return;
    }

    // Supprimer le caractère ':' au début du message
    message = message.substr(1);

    // Vérifier si le nickname existe parmi les clients
    Client *recipient = NULL;
    for (size_t i = 0; i < _clients.size(); i++) 
    {
        if (_clients[i].getNickname() == nickname) 
        {
            recipient = &_clients[i];
            break;
        }
    }

    // Si le destinataire existe, envoyer le message privé
    if (recipient) 
    {
        std::string privateMessage = "PRIVMSG " + client->getNickname() + " : " + message + "\n";
        send(recipient->getFd(), privateMessage.c_str(), privateMessage.size(), 0);
        send(client->getFd(), (MAGENTA "~> Message sent to " + nickname + "\n" + RESET).c_str(), 32 + nickname.size(), 0);
    } 
    else 
    {
        send(client->getFd(), CYAN "~> Nickname not found\n" RESET, 34, 0);
    }
}
void Server::handleMessage(int clientFd) 
{
    char buffer[512];
    ssize_t bytes_received = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received <= 0) 
    {
        std::cout << "Client déconnecté: FD " << clientFd << std::endl;
        removeClient(clientFd);
        return;
    }

    buffer[bytes_received] = '\0';
    std::string message(buffer);
    message.erase(message.find_last_not_of(" \n\r\t") + 1);

    if (message.empty())
    {
        return;
    }

    Client* client = getClientByFd(clientFd);
    if (!client) 
    {
        return;
    }

    std::istringstream iss(message);
    std::string command;
    iss >> command;

    if (command == "JOIN") 
    {
        std::string channel;
        iss >> channel;
        if (!channel.empty())
        {
            handleJoinCommand(_channelManager, *client, channel);
        }
        else
        {
            sendIrcResponse(*client, "461 JOIN :Not enough parameters");
        }
    }
    else if (command == "PART") 
    {
        std::string channel, reason;
        iss >> channel;
        std::getline(iss, reason);
        if (!channel.empty())
        {
            handlePartCommand(_channelManager, *client, channel, reason);
        }
        else
        {
            sendIrcResponse(*client, "461 PART :Not enough parameters");
        }
    }
    else if (command == "PRIVMSG") 
    {
        std::string target, msg;
        iss >> target;
        std::getline(iss, msg);
        if (!target.empty() && !msg.empty())
        {
            msg.erase(0, msg.find_first_not_of(" :"));
            handlePrivmsg(*client, target, msg);
        }
        else
        {
            sendIrcResponse(*client, "461 PRIVMSG :Not enough parameters");
        }
    }
    else if (command == "KICK") 
    {
        std::string channel, target, reason;
        iss >> channel >> target;
        std::getline(iss, reason);
        if (!channel.empty() && !target.empty())
        {
            handleKickCommand(_channelManager, *client, channel, target, reason);
        }
        else
        {
            sendIrcResponse(*client, "461 KICK :Not enough parameters");
        }
    }
    else if (command == "INVITE") 
    {
        std::string target, channel;
        iss >> target >> channel;
        if (!target.empty() && !channel.empty())
        {
            handleInviteCommand(_channelManager, *client, channel, target);
        }
        else
        {
            sendIrcResponse(*client, "461 INVITE :Not enough parameters");
        }
    }
    else if (command == "TOPIC") 
    {
        std::string channel, topic;
        iss >> channel;
        std::getline(iss, topic);
        if (!channel.empty())
        {
            handleTopicCommand(_channelManager, *client, channel, topic);
        }
        else
        {
            sendIrcResponse(*client, "461 TOPIC :Not enough parameters");
        }
    }
    else if (command == "MODE") 
    {
        std::string channel, mode, arg;
        iss >> channel >> mode;
        std::getline(iss, arg);
        if (!channel.empty())
        {
            handleModeCommand(_channelManager, *client, channel, mode, arg);
        }
        else
        {
            sendIrcResponse(*client, "461 MODE :Not enough parameters");
        }
    }
    else 
    {
        sendIrcResponse(*client, "421 " + command + " :Unknown command");
    }
}

