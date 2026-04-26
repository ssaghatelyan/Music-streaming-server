#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <thread>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <dirent.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 8080

std::map<std::string, std::string> users;

std::string hashPassword(const std::string& pass) {
    uint64_t hash = 14695981039346656037ULL;
    for (unsigned char c : pass) {
        hash ^= c;
        hash *= 1099511628211ULL;
    }

    hash ^= 0xDEADBEEFCAFEBABEULL;
    hash *= 1099511628211ULL;
    return std::to_string(hash);
}

void loadUsers(const std::string& file) {
    std::ifstream f(file);
    std::string line;

    while (std::getline(f, line)) {
        std::stringstream ss(line);
        std::string user, hash;

        if (std::getline(ss, user, ':') && std::getline(ss, hash))
            users[user] = hash;
    }
}

void saveUsers(const std::string& file) {
    std::ofstream f(file);
    for (auto& u : users)
        f << u.first << ":" << u.second << "\n";
}

bool signupUser(const std::string& user, const std::string& pass) {
    if (users.count(user))
        return false;

    users[user] = hashPassword(pass);
    saveUsers("users.txt");
    return true;
}

bool loginUser(const std::string& user, const std::string& pass) {
    return users.count(user) && users[user] == hashPassword(pass);
}

void sendAll(int client, const void* data, size_t size) {
    const char* ptr = (const char*)data;
    size_t sent = 0;

    while (sent < size) {
        ssize_t s = send(client, ptr + sent, size - sent, 0);
        if (s <= 0) return;
        sent += s;
    }
}

void sendString(int client, const std::string& str) {
    size_t len = str.size();
    sendAll(client, &len, sizeof(len));
    sendAll(client, str.c_str(), len);
}

bool isSafeFilename(const std::string& name) {
    if (name.empty()) return false;
    if (name.find("..") != std::string::npos) return false;
    if (name.find('/') != std::string::npos) return false;
    if (name.find('\\') != std::string::npos) return false;
    return true;
}

void sendSong(int client, const std::string& name) {

    if (!isSafeFilename(name)) {
        size_t err = 0;
        sendAll(client, &err, sizeof(err));
        std::cerr << "Path traversal attempt blocked: " << name << "\n";
        return;
    }

    std::string path = "songs/" + name;

    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        size_t err = 0;
        sendAll(client, &err, sizeof(err));
        return;
    }

    struct stat st;
    fstat(fd, &st);

    size_t size = st.st_size;
    sendAll(client, &size, sizeof(size));

    off_t offset = 0;
    while (offset < (off_t)size) {
        ssize_t sent = sendfile(client, fd, &offset, size - offset);
        if (sent <= 0) break;
    }

    close(fd);
}

std::string recvLine(int client) {
    std::string line;
    char c;

    while (true) {
        int bytes = recv(client, &c, 1, 0);
        if (bytes <= 0) return "";
        if (c == '\n') break;
        line += c;
    }

    return line;
}

void handleClient(int client) {
    bool logged = false;

    while (true) {

        std::string line = recvLine(client);
        if (line.empty()) break;

        std::stringstream ss(line);
        std::string cmd, a1, a2;
        ss >> cmd;

        if (cmd == "play") {
            std::getline(ss, a1);
            if (!a1.empty() && a1[0] == ' ')
                a1 = a1.substr(1);
        } else {
            ss >> a1 >> a2;
        }

        if (cmd == "signup") {
            std::string res = signupUser(a1, a2) ? "SIGNUP_OK" : "USER_EXISTS";
            sendString(client, res);
        }

        else if (cmd == "login") {
            std::string res = loginUser(a1, a2) ? "LOGIN_OK" : "LOGIN_FAILED";
            if (res == "LOGIN_OK") logged = true;
            sendString(client, res);
        }

        else if (cmd == "list") {

            DIR* dir = opendir("songs");
            if (!dir) {
                sendString(client, "(no songs directory)\n");
                continue;
            }

            std::string result;
            struct dirent* file;
            while ((file = readdir(dir)) != NULL) {
                std::string name = file->d_name;
                if (name == "." || name == "..") continue;
                result += name + "\n";
            }

            closedir(dir);
            sendString(client, result);
        }

        else if (cmd == "play") {
            if (!logged) {
                size_t err = 0;
                sendAll(client, &err, sizeof(err));
                continue;
            }
            sendSong(client, a1);
        }
    }

    close(client);
}

int main() {
    loadUsers("users.txt");

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

<<<<<<< HEAD
=======
    // FIX #7: SO_REUSEADDR — чтобы порт не зависал после перезапуска
>>>>>>> 27b798c (.)
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&addr, sizeof(addr));
    listen(serverSocket, 5);

    std::cout << "Server started on port " << PORT << "\n";

    while (true) {
        int client = accept(serverSocket, NULL, NULL);
        if (client < 0) continue;
<<<<<<< HEAD
=======

        // FIX #3: Запускаем клиента в отдельном потоке
>>>>>>> 27b798c (.)
        std::thread(handleClient, client).detach();
    }
}