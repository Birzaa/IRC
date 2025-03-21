#include "MockServer.hpp"
#include <iostream>

void MockServer::sendMessage(const std::string& user, const std::string& message) {
    std::cout << "Message envoyé à " << user << ": " << message << std::endl;
}