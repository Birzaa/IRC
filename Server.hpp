#ifndef SERVER_HPP
#define SERVER_HPP

class Server 
{
	private :
		int _serverFd, _port;
	
	public : 
		Server();
		~Server();
};

#endif // SERVER_HPP