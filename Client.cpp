#include "Client.hpp"

Client::Client()
{
	this->_fd = -1;
	this->_isAuthenticated = false;
	this->authAttempts = 3;
	this->_hostname = "";
	this->_ipClient = "";
	this->_nickname = "";
	this->_realname = "";
	this->_servername = "";
	this->_username = "";
	this->_lastPong = time(NULL);

}

Client::~Client()
{}

Client& Client::operator=(const Client& other) 
{
	if (this != &other)
	{
		this->_fd = other._fd;
		this->_ipClient = other._ipClient;
		this->_isAuthenticated = other._isAuthenticated;
		this->authAttempts = other.authAttempts;
		this->_nickname = other._nickname;
		this->_username = other._username;
		this->_hostname = other._hostname;
		this->_servername = other._servername;
		this->_realname = other._realname;
	}
	return *this;
}

Client::Client(const Client& other)
{
	this->_fd = other._fd;
	this->_ipClient = other._ipClient;
	this->_isAuthenticated = other._isAuthenticated;
	this->authAttempts = other.authAttempts;
	this->_nickname = other._nickname;
	this->_username = other._username;
	this->_hostname = other._hostname;
	this->_servername = other._servername;
	this->_realname = other._realname;
}


// Getter

int Client::getFd() const
{
	return _fd;
}

void Client::setFd(int fd)
{
	_fd = fd;
}

bool Client::getAuthenticated() const
{
	return this->_isAuthenticated;
}

std::string Client::getNickname() const 
{
    return this->_nickname;
}

std::string Client::getUsername() const
{
	return this->_username;
}

std::string Client::getHostname() const
{
	return this->_hostname;
}

std::string Client::getServername() const
{
	return this->_servername;
}

std::string Client::getRealname() const
{
	return this->_realname;
}


// Setter

std::string Client::getIpClient() const
{
	return this->_ipClient;
}

void Client::setIpClient(const std::string& ipClient)
{
	this->_ipClient = ipClient;
}

void Client::setAuthenticated(bool isAuthenticated)
{
	this->_isAuthenticated = isAuthenticated;
}

void Client::setNickname(const std::string& nickname) 
{
    this->_nickname = nickname;
}

void Client::setUsername(const std::string& username)
{
	this->_username = username;
}

void Client::setHostname(const std::string& hostname)
{
	this->_hostname = hostname;
}

void Client::setServername(const std::string& servername)
{
	this->_servername = servername;
}

void Client::setRealname(const std::string& realname)
{
	this->_realname = realname;
}


time_t Client::getLastPong() const 
{
    return _lastPong;
}

void Client::setLastPong(time_t time) 
{
    _lastPong = time;
}