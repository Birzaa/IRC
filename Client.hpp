#ifndef A7802908_8117_4429_B353_4563C963B63E
#define A7802908_8117_4429_B353_4563C963B63E
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


	public:
		Client();
		~Client();
		Client& operator=(const Client &rhs);
		Client(const Client& other);
		
		int authAttempts;

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


#endif /* A7802908_8117_4429_B353_4563C963B63E */
