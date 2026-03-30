#include "header.hpp"

Auth::Auth(const std::string& file) : dbFile(file) {
    load();
}

void Auth::load() {
    std::ifstream file(dbFile);
    std::string line;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string user, hash;

        if (std::getline(ss, user, ':') && std::getline(ss, hash))
            users[user] = hash;
    }
}

void Auth::save() {
    std::ofstream file(dbFile);

    for (auto &u : users)
        file << u.first << ":" << u.second << "\n";
}

std::string Auth::signup(const std::string& user, const std::string& pass) {
    if (users.count(user))
        return "USER_EXISTS";

    users[user] = hashPassword(pass);
    save();

    return "SIGNUP_OK";
}

std::string Auth::login(const std::string& user, const std::string& pass) {
    if (!users.count(user))
        return "LOGIN_FAILED";

    if (verifyPassword(pass, users[user]))
        return "LOGIN_OK";

    return "LOGIN_FAILED";
}