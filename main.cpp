#include "Server.hpp"
#define RED "\033[31m"
#define RESET "\033[0m"

int main (int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << RED << "Error : " << "./irc <port> <password>" << RESET << std::endl;
		return 1;
	}
	
	Server server(av[1], av[2]);
	try
	{
		signal(SIGINT, Server::signalHandler); // catch CTRL+C
		signal(SIGQUIT, Server::signalHandler); // catch (CTRL+\)
		server.initServer();
	}
	catch(const std::exception& e)
	{
		server.closeFds();
		std::cerr << e.what() << '\n';
		return 1;
	}
	
	return 0;
}	