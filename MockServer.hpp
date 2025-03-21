#ifndef MOCKSERVER_HPP
#define MOCKSERVER_HPP

#include <string>
#include <iostream>

class MockServer {
public:
    void sendMessage(const std::string& user, const std::string& message) {
        std::cout << "Message envoyé à " << user << ": " << message << std::endl;
    }
};

#endif