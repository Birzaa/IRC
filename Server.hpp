#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <map>
#include <poll.h>
#include "Client.hpp"
#include "ChannelManager.hpp"

class Server {
private:
    int _serverFd;
    int _port;
    std::string _password;
    std::vector<Client> _clients;
    std::vector<pollfd> _fds;
    ChannelManager _channelManager;
    static bool signal;
    void initSocket();
    void startServer();
    void acceptClient();
    void handleMessage(int client_fd);
    void removeClient(int client_fd);

public:
    Server(const std::string &port, const std::string &password);
    ~Server();

    void initServer();
    void closeFds();
    static void signalHandler(int signal);
    Client* getClientByFd(int fd);
    Client* getClientByNickname(const std::string& nickname);
    ChannelManager& getChannelManager();  
    bool isRegistered(Client *client);

    // Command handlers
    void handleNick(Client *client, std::istringstream &iss);
    void handleUser(Client *client, std::istringstream &iss);
    void handlePrivmsg(Client *client, std::istringstream &iss);
    void handleJoin(Client *client, std::istringstream &iss);
    void handlePart(Client *client, std::istringstream &iss);
    void handleKick(Client *client, std::istringstream &iss);
    void handleInvite(Client *client, std::istringstream &iss);
    void handleTopic(Client *client, std::istringstream &iss);
    void handleMode(Client *client, std::istringstream &iss);
    void sendToClient(int fd, const std::string& message);
    void sendToClient(const std::string& nickname, const std::string& message);
    void sendToChannel(const std::string& channelName, const std::string& message);

};

#endif