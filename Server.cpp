#include "Server.hpp"

Server::Server(const std::string &port, const std::string &password)
{
	this->_port = atoi(port.c_str());
	this->_password = password;
}

Server::~Server()
{
	for (size_t i = 0; i < _clients.size(); i++) 
    	delete _clients[i]; 

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
		std::cout << ORANGE << "Client : " << this->_clients[i]->getFd() << " Disconnected" << RESET << std::endl;
		close(this->_clients[i]->getFd());
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
		else if (command == "JOIN")
			createChannel(iss, client);
		else if (command == "PART")
			handlePart(client, iss);
		else if (command == "QUIT")
			handleQuit(client);
		else if (command == "KICK")
			handleKick(client, iss);
	}

}

void Server::removeClient(int clientFd) 
{
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
        if (_clients[i]->getFd() == fd) 
            return _clients[i];  // Retourne vers le client trouvé
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
        if (_clients[i]->getNickname() == nickname) 
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
	// return !client->getNickname().empty() && !client->getUsername().empty() && !client->getHostname().empty() && !client->getServername().empty() && !client->getRealname().empty();
	(void)client;
	return 1;
}

void Server::handlePrivmsg(Client *client, std::istringstream &iss)
{
    std::string target, message;

    if (!isRegistered(client))
    {
        send(client->getFd(), CYAN "~> You must set your nickname and user information first\n" RESET, 69, 0);
        return;
    }

    // Extraire la cible (nickname ou channel)
    iss >> target;
    
    // Vérification si le message est vide
    std::getline(iss, message);
    message.erase(0, message.find_first_not_of(" \t")); // Supprimer les espaces/tabulations au début

    if (target.empty() || message.empty())
    {
        send(client->getFd(), CYAN "~> Usage: PRIVMSG <target> :<message>\n" RESET, 46, 0);
        return;
    }

    // Vérifier le format du message
    if (message[0] != ':')
    {
        send(client->getFd(), CYAN "~> Message must start with ':'\n" RESET, 38, 0);
        return;
    }
    message = message.substr(1); // Supprimer le ':'

    // CAS 1: Message à un channel (commence par #)
    if (target[0] == '#')
    {
        std::map<std::string, Channel>::iterator it = _channels.find(target);
        if (it == _channels.end())
        {
            send(client->getFd(), CYAN "~> Channel not found\n" RESET, 35, 0);
            return;
        }

        // Vérifier que l'expéditeur est dans le channel
        bool inChannel = false;
        const std::vector<Client*>& members = it->second.getClients();
        for (size_t i = 0; i < members.size(); ++i)
        {
            if (members[i]->getFd() == client->getFd())
            {
                inChannel = true;
                break;
            }
        }

        if (!inChannel)
        {
            send(client->getFd(), CYAN "~> You're not in this channel\n" RESET, 42, 0);
            return;
        }

        // Envoyer le message à tous les membres du channel
        std::string channelMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + 
                                " PRIVMSG " + target + " :" + message + "\r\n";
        
        for (size_t i = 0; i < members.size(); ++i)
        {
            if (members[i]->getFd() != client->getFd()) // Ne pas renvoyer à l'expéditeur
            {
                send(members[i]->getFd(), channelMsg.c_str(), channelMsg.size(), 0);
            }
        }
        send(client->getFd(), (MAGENTA "~> Message sent to " + target + "\n" RESET).c_str(), 32 + target.size(), 0);
    }
    // CAS 2: Message privé
    else
    {
        Client *recipient = NULL;
        for (size_t i = 0; i < _clients.size(); i++) 
        {
            if (_clients[i]->getNickname() == target) 
            {
                recipient = _clients[i];
                break;
            }
        }

        if (recipient) 
        {
            std::string privateMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + 
                                   " PRIVMSG " + target + " :" + message + "\r\n";
            send(recipient->getFd(), privateMsg.c_str(), privateMsg.size(), 0);
            send(client->getFd(), (MAGENTA "~> Message sent to " + target + "\n" RESET).c_str(), 32 + target.size(), 0);
        } 
        else 
        {
            send(client->getFd(), CYAN "~> Nickname not found\n" RESET, 34, 0);
        }
    }
}



//                                Channel

void Server::createChannel(std::istringstream& iss, Client* client) 
{
	std::string channelName;
    iss >> channelName;

	if (!isRegistered(client))
    {
        send(client->getFd(), CYAN "~> You must set your nickname and user information first\n" RESET, 69, 0);
        return;
    }

	if (channelName.empty() || channelName[0] != '#')
	{
		send(client->getFd(), CYAN "~> Usage: CREATE <#channel_name>\n" RESET, 44, 0);
        return;
	}

	// Channel existe deja
    if (_channels.find(channelName) != _channels.end()) 
	{
		std::map<std::string, Channel>::iterator it = _channels.find(channelName);
		it->second.addClient(client);
        return;
    }
    
    std::pair<std::map<std::string, Channel>::iterator, bool> result = _channels.insert(std::make_pair(channelName, Channel(channelName)));

    if (!result.second) 
	{
        send(client->getFd(), CYAN "~> Failed to create channel\n" RESET, 39, 0);
        return;
    }
    

    // Ajouter le client au channel
	std::map<std::string, Channel>::iterator it = _channels.find(channelName);
    it->second.addClient(client);

    std::cout << GREEN << "Channel created: " << channelName << RESET << std::endl;
}

void Server::handlePart(Client* client, std::istringstream& iss) 
{
    std::string channelName;
    iss >> channelName;

    if (channelName.empty()) 
	{
        send(client->getFd(), CYAN "~> Usage: PART <channel>\n" RESET, 36, 0);
        return;
    }

    // Vérifier si le canal existe
    std::map<std::string, Channel>::iterator it = _channels.find(channelName);
    if (it == _channels.end()) 
	{
        send(client->getFd(), CYAN "~> Channel does not exist\n" RESET, 37, 0);
        return;
    }

    // Retirer le client du canal
    it->second.removeClient(client);

    // Envoyer un message de confirmation
	std::string msg = MAGENTA "~> You left the channel: " + channelName + "\n"  RESET;
    send(client->getFd(), msg.c_str(), 40 + channelName.size(), 0);

    // Supprimer le canal s'il est vide
    if (it->second.getClients().empty()) 
	{
        _channels.erase(it);
        std::cout << RED << "Channel deleted: " << channelName << RESET << std::endl;
    }
}

void Server::handleQuit(Client* client) 
{
    std::map<std::string, Channel>::iterator it = _channels.begin();
    while (it != _channels.end()) 
	{
        it->second.removeClient(client);
        if (it->second.getClients().empty()) 
		{
            std::cout << RED << "Channel deleted: " << it->first << RESET << std::endl;
            _channels.erase(it++);
        } 
		else 
            ++it;
    }
    removeClient(client->getFd());
}

void Server::handleKick(Client* client, std::istringstream& iss) 
{
    std::string channelName, targetNick, reason;
    iss >> channelName >> targetNick;
    std::getline(iss, reason); // Capture ":Raison"

    // Nettoyer la raison
    if (!reason.empty() && reason[0] == ' ')
        reason = reason.substr(1);
    if (!reason.empty() && reason[0] == ':')
        reason = reason.substr(1);

    // Vérifier que le channel existe
    if (_channels.find(channelName) == _channels.end()) 
    {
        std::string msg = std::string(CYAN) + channelName + " :No such channel\n" + RESET;
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    Channel& channel = _channels[channelName];

    // Vérifier que le demandeur est opérateur
    bool isOp = false;
    const std::vector<Client*>& operators = channel.getOperators();
    for (std::vector<Client*>::const_iterator it = operators.begin(); it != operators.end(); ++it)
    {
        if ((*it)->getFd() == client->getFd()) 
        {
            isOp = true;
            break;
        }
    }

    if (!isOp) 
    {
        std::string msg = std::string(CYAN) + channelName + " :You're not channel operator\n" + RESET;
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // Trouver la cible
    Client* target = NULL;
    const std::vector<Client*>& clients = channel.getClients();
    for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if ((*it)->getNickname() == targetNick) 
        {
            target = *it;
            break;
        }
    }

    if (!target) 
    {
        std::string msg = std::string(CYAN) + targetNick + " :No such nickname\n" + RESET;
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    // Exécuter le kick
    std::string kickMsg = ":" + client->getNickname() + " KICK " + channelName + " " + targetNick + " :" + (reason.empty() ? "No reason" : reason) + "\r\n";
    
    // Envoyer à tous les membres du channel
    for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it)
    {
        send((*it)->getFd(), kickMsg.c_str(), kickMsg.size(), 0);
    }

    // Retirer la cible du channel
    channel.removeClient(target);
}