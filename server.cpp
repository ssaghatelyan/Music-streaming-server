#include "header.hpp"

#include <dirent.h>

std::string getSongsList() {
    DIR* dir = opendir("songs");
    if (!dir) return "NO_FOLDER\n";

    struct dirent* entry;
    std::string list = "";

    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;

        if (name == "." || name == "..")
            continue;

        list += name + "\n";
    }

    closedir(dir);
    return list;
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);

    std::cout << "Server started...\n";

    Auth auth("users.db");

    while (true) {
        int client = accept(server_fd, nullptr, nullptr);

        while (true) {
            char buffer[1024] = {0};
            int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);

            if (bytes <= 0)
                break;

            std::string cmd(buffer, bytes);

            std::stringstream ss(cmd);
            std::string action, user, pass;

            ss >> action >> user >> pass;

            std::string response = "UNKNOWN";

            if (action == "signup")
                response = auth.signup(user, pass);

            else if (action == "login")
                response = auth.login(user, pass);

            else if (action == "list") {
                response = getSongsList();
            }

            else if (action == "play") {
                std::string path = "songs/" + user;

                std::ifstream file(path, std::ios::binary);

                if (!file) {
                    std::string err = "FILE_NOT_FOUND\n";
                    send(client, err.c_str(), err.size(), 0);
                    continue;
                }

                char buf[1024];
                while (file.read(buf, sizeof(buf))) {
                    send(client, buf, sizeof(buf), 0);
                }

                send(client, buf, file.gcount(), 0);
                continue;
            }

            response += "\n";
            send(client, response.c_str(), response.size(), 0);
        }

        close(client);
    }
}