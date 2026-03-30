#ifndef HEADER_HPP
#define HEADER_HPP

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <openssl/sha.h>

#define PORT 8080

std::string hashPassword(const std::string& input);
bool verifyPassword(const std::string& password, const std::string& hash);

class Auth {
private:
    std::string dbFile;
    std::map<std::string, std::string> users;

public:
    Auth(const std::string& file);

    void load();
    void save();

    std::string signup(const std::string& user, const std::string& pass);
    std::string login(const std::string& user, const std::string& pass);
};

#endif