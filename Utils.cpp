#include "Utils.hpp"

std::string	printTime() 
{
	time_t		currentEpochTime;
	struct tm	localTimeStruct;
	char		buffer[9];

	time(&currentEpochTime);
	localTimeStruct = *localtime(&currentEpochTime);
	std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &localTimeStruct);

	std::ostringstream oss;
	oss << BLUE << "[" << buffer << "] " << RESET;
	return (oss.str());
}

bool isValidIRCPort(int port) 
{
    return (port > 0 && port <= 65535) && 
          (port == 6667 || port == 6697 || 
           (port >= 6665 && port <= 6669) || 
           port >= 7000);
}

int	checkAtoi(const std::string &number) {
	char *tracker;
	long goodValue;

	goodValue = strtol(number.c_str(), &tracker, 10);

	// Check if the entire string was parsed
	if (*tracker != '\0' && *tracker != '\n')
		throw std::runtime_error(RED "[ERROR] Given port is invalid !" RESET);

	// check if out of bounds
	if (errno == ERANGE)
		throw std::runtime_error(RED "[ERROR] Your argument is out-of-bounds..." RESET);
	return (goodValue);
}