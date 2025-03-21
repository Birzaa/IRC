#include "MockServer.hpp"

void sendMessage(const std::string& user, const std::string& message) {
    MockServer mockServer;
    mockServer.sendMessage(user, message);
}