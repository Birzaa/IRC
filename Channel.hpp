#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>

class Channel 
{
private:
    std::string name;
    std::vector<std::string> users;
    std::vector<std::string> operators;
    std::string topic;
    bool inviteOnly;
    bool topicRestricted;
    std::string password;
    size_t userLimit;
    std::vector<std::string> invitedUsers;

public:
    Channel(const std::string& name);
    
    void addUser(const std::string& user);
    void removeUser(const std::string& user);
    bool hasUser(const std::string& user) const;
    bool isEmpty() const;
    
    void setTopic(const std::string& newTopic);
    std::string getTopic() const;
    
    void addOperator(const std::string& user);
    void removeOperator(const std::string& user);
    bool isOperator(const std::string& user) const;
    
    void setMode(char mode, bool enable, const std::string& arg = "");
    bool checkInviteOnly() const;
    bool checkTopicRestricted() const;
    bool checkPassword(const std::string& pass) const;
    bool checkUserLimit() const;
    
    void addInvitedUser(const std::string& user);
    bool isInvited(const std::string& user) const;
    
    const std::vector<std::string>& getUsers() const;
    std::string getPassword() const;
    size_t getUserLimit() const;
};

#endif