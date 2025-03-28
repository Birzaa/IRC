#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in
#include <unistd.h> // close
#include <vector>
#include <poll.h>
#include <fcntl.h>
#include <cstdlib> // atoi
#include <stdexcept> // runtime_error
#include <arpa/inet.h> // inet_ntoa
#include <signal.h> // signal
#include <algorithm> // remove
#include <sstream> // istringstream
#include <map>


#define GREEN   "\033[0;32m"
#define RED     "\033[0;31m"
#define YELLOW  "\033[0;33m"
#define CYAN    "\033[1;36m"
#define MAGENTA "\033[0;35m"
#define ORANGE  "\033[38;5;216m"
#define BLUE    "\033[0;34m"
#define RESET   "\033[0m"
#define BOLD_RED "\033[1;31m"


#include "Client.hpp"
#include "Channel.hpp"

class Server 
{
	private :
		int _serverFd, _port;
		std::string _password;
		std::vector<Client*> _clients; // all clients
		std::vector<pollfd> _fds; // list of file descriptors

		void initSocket();
		void startServer();
		void acceptClient();
		void handleMessage(int client_fd);
        void handlePass(Client* client, std::istringstream& iss);
		void removeClient(int client_fd);

		static const std::string VALID_COMMANDS[11];

		// Channel
		std::map<std::string, Channel> _channels;

	public:
		Server(const std::string &port, const std::string &password);
		~Server();
		
		static bool signal;

		// Server methods
		void closeFds();
		void initServer();
		static void signalHandler(int signal);
		Client* getClientByFd(int fd);

		// Client methods
		void handleNick(Client *client, std::istringstream &iss);
		void handleUser(Client *client, std::istringstream &iss);
		void handlePrivmsg(Client *client, std::istringstream &iss);
		bool isRegistered(Client *client);

		// Channel
		void createChannel(std::istringstream& channelName, Client* client);
		void handlePart(Client* client, std::istringstream& iss);
		void handleQuit(Client* client);

		//Operator
		void handleKick(Client* client, std::istringstream& iss);
        void handleInvite(Client* client, std::istringstream& iss);
        void handleTopic(Client* client, std::istringstream& iss);
        void handleMode(Client* client, std::istringstream& iss);

};

#endif // SERVER_HPP