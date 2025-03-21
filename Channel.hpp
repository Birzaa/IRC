#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>

class Channel 
{
    private:
        std::string name;
        std::vector<std::string> users;
        std::vector<std::string> operators; // Liste des opérateurs
        std::string topic;

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

        const std::vector<std::string>& getUsers() const;
};

#endif