#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <map>

class Channel
{
private:
    std::string _name;
    std::string _topic;
    std::string _key;
    std::vector<std::string> _users;
    std::vector<std::string> _operators;
    std::vector<std::string> _invited;
    bool _inviteOnly;
    bool _topicProtected;
    size_t _userLimit;

public:
    Channel(std::string const& name);
    ~Channel();

    // Getters
    std::string getName() const;
    std::string getTopic() const;
    std::string getKey() const;
    std::vector<std::string> getUsers() const;
    bool isInviteOnly() const;
    bool isTopicProtected() const;
    size_t getUserLimit() const;
    bool hasUser(std::string const& nickname) const;
    bool isOperator(std::string const& nickname) const;
    bool isInvited(std::string const& nickname) const;
    bool isEmpty() const;
    bool hasKey() const;
    bool hasUserLimit() const;

    // Setters
    void setTopic(std::string const& topic);
    void setKey(std::string const& key);
    void setInviteOnly(bool inviteOnly);
    void setTopicProtected(bool topicProtected);
    void setUserLimit(size_t limit);

    // User management
    void addUser(std::string const& nickname);
    void removeUser(std::string const& nickname);
    void addOperator(std::string const& nickname);
    void removeOperator(std::string const& nickname);
    void addInvited(std::string const& nickname);
    void removeInvited(std::string const& nickname);
    void removeKey();
    void removeUserLimit();
};

#endif