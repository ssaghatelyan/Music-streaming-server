#include "server.hpp"

bool user_exists(const std::string& username) {
    std::ifstream file("users.txt");
    if (!file)
        std::cout << "File not found\n";
    std::string u, p;

    while (file >> u >> p) {
        if (u == username)
            return true;
    }

    return false;
}

bool authenticate(const std::string& username, const std::string& password) {
    std::ifstream file("users.txt");
    if (!file)
        std::cout << "File not found\n";
    std::string u, p;

    while (file >> u >> p) {
        if (u == username && p == password)
            return true;
    }

    return false;
}

void add_user(const std::string& username, const std::string& password) {
    std::ofstream file("users.txt", std::ios::app);
    file << username << " " << password << "\n";
}

int main() {
    int option;
    std::string username, password;

    std::cout << "Choose option:\n1 LOGIN\n2 SIGNUP\n";
    std::cout << "Enter option, username, password: ";
    std::cin >> option >> username >> password;

    if (option == 1) {
        if (!user_exists(username)) {
            std::cout << "ERROR User not found\n";
        } else if (!authenticate(username, password)) {
            std::cout << "ERROR Wrong password\n";
        } else {
            std::cout << "OK Logged in\n";
        }
    } else if (option == 2) {
        if (user_exists(username)) {
            std::cout << "ERROR User already exists\n";
        } else {
            add_user(username, password);
            std::cout << "OK Registered\n";
        }
    } else {
        std::cout << "ERROR Invalid option\n";
    }

    return 0;
}