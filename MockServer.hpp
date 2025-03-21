#ifndef MOCKSERVER_HPP
#define MOCKSERVER_HPP

#include <string>

class MockServer {
public:
    void sendMessage(const std::string& user, const std::string& message);
};

#endif