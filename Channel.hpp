#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>

#define GREEN   "\033[0;32m"
#define RED     "\033[0;31m"
#define YELLOW  "\033[0;33m"
#define CYAN    "\033[1;36m"
#define MAGENTA "\033[0;35m"
#define ORANGE  "\033[38;5;216m"
#define BLUE    "\033[0;34m"
#define RESET   "\033[0m"
#define BOLD_RED "\033[1;31m"

class Channel 
{
	private:
		std::string _name;
		std::string _topic;
		std::vector<Client*> _clients;
		std::vector<Client*> _operators;
		bool _inviteOnly;
		std::string _password;
		int _maxClients;

		void alertAll(const std::string& nickname, const std::string& msg);

	public:
		Channel();
		Channel(const std::string& name);
		~Channel();

		// Gestion des clients
		void addClient(Client* client);
		void removeClient(Client* client);
		const std::vector<Client*>& getClients() const;

		// Gestion des messages
		void broadcastMessage(const std::string& sender, const std::string& message);

		// Gestion des opérateurs
		void kickClient(Client* kicker, Client* target, const std::string& reason);
		bool isOperator(Client* client) const;
		void addOperator(Client* client);
		void removeOperator(Client* client);
		const std::vector<Client*>& getOperators() const;

		
};

#endif // CHANNEL_HPP
