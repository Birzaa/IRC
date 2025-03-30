#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cerrno>
#include <cstdlib> 

#define GREEN   "\033[0;32m"
#define RED     "\033[0;31m"
#define YELLOW  "\033[0;33m"
#define CYAN    "\033[1;36m"
#define MAGENTA "\033[0;35m"
#define ORANGE  "\033[38;5;216m"
#define BLUE    "\033[0;34m"
#define RESET   "\033[0m"
#define BOLD_RED "\033[1;31m"

std::string	printTime();
bool isValidIRCPort(int port);
int	checkAtoi(const std::string &number);

#endif