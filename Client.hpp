#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

class Client
{
	private:
		int _fd;
		std::string _ipClient;
		bool _isAuthenticated;
		std::string _nickname;
		std::string _username;
		std::string _hostname;
		std::string _servername;
		std::string _realname;
		time_t _lastPong;


	public:
		Client();
		~Client();
		Client& operator=(const Client &rhs);
		Client(const Client& other);
		
		int authAttempts;
		
		time_t getLastPong() const;
    	void setLastPong(time_t time);

		// Getter
		int getFd() const;
		std::string getIpClient() const;
		bool getAuthenticated() const;
		std::string getNickname() const;
		std::string getUsername() const;
		std::string getHostname() const;
		std::string getServername() const;
		std::string getRealname() const;


		// Setter
		void setFd(int fd);
		void setIpClient(const std::string& ipClient);
    	void setAuthenticated(bool isAuthenticated);
		void setNickname(const std::string& nickname);
		void setUsername(const std::string& username);
		void setHostname(const std::string& hostname);
		void setServername(const std::string& servername);
		void setRealname(const std::string& realname);
};


#endif // CLIENT_HPP