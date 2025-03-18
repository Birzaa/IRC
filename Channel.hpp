#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <stdexcept> // Pour std::runtime_error

class Channel 
{
    private:
        std::string name;
        std::vector<std::string> users;
        std::vector<std::string> operators; // Liste des opérateurs
        std::string topic;

    public:
        Channel();
        ~Channel();
        Channel(const std::string& name);
        Channel(const Channel& other); // Constructeur de copie
        Channel& operator=(const Channel& other); // Opérateur d'affectation

        void addUser(const std::string& user);
        void removeUser(const std::string& user);
        bool hasUser(const std::string& user) const;
        void setTopic(const std::string& newTopic);
        std::string getTopic() const;

        // Gestion des opérateurs
        void addOperator(const std::string& user);
        void removeOperator(const std::string& user);
        bool isOperator(const std::string& user) const;
};

#endif