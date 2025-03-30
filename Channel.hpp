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
		std::string _password;
		int _maxClients;
        std::vector<std::string> _invited; // Liste des clients invités
        bool _topicRestricted; // Mode +t
		time_t _creationTime;
        
	protected:
		bool _inviteOnly;

	public:
		Channel();
		Channel(const std::string& name);
		~Channel();

		void alertAll(const std::string& nickname, const std::string& msg);
		// Gestion des clients
		void broadcast(const std::string& message);
		void broadcast2(const std::string& message, Client* exclude = NULL);
        void addClient(Client* client, const std::string& password);
		void removeClient(Client* client);
		const std::vector<Client*>& getClients() const;
		bool isMember(Client* client) const;
		Client* getClient(const std::string& nickname) const;
		void sendIrcReply(int code, Client* client, const std::string& arg1, const std::string& arg2);
		void sendJoinReply(Client* client);

		// Gestion des messages
		void broadcastMessage(const std::string& sender, const std::string& message);

		// Gestion des opérateurs
		void kickClient(Client* kicker, Client* target, const std::string& reason);
		bool isOperator(Client* client) const;
		void addOperator(Client* client);
		void removeOperator(Client* client);
		const std::vector<Client*>& getOperators() const;

		void setTopic(const std::string& topic);
        const std::string& getTopic() const;
        void setInviteOnly(bool inviteOnly);
        bool getInviteOnly() const;
        void setPassword(const std::string& password);
        const std::string& getPassword() const;
        void setMaxClients(int maxClients);
        int getMaxClients() const;
        void addInvited(const std::string& nickname);
        bool isInvited(const std::string& nickname) const;
        bool getTopicRestricted() const;
        void setTopicRestricted(bool restricted);
};

#endif // CHANNEL_HPP
