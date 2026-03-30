#include "header.hpp"
#include <termios.h>

std::string getPassword() {
    std::string password;

    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);

    newt = oldt;
    newt.c_lflag &= ~ECHO;

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::cin >> password;

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl;

    return password;
}

std::string recvAll(int sock) {
    char buffer[1024];
    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);

    if (bytes <= 0)
        return "ERROR";

    return std::string(buffer, bytes);
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (sockaddr*)&addr, sizeof(addr));

    bool loggedIn = false;

    while (true) {
        if (!loggedIn) {
            std::cout << "\n1. Login\n2. Signup\n3. Exit\nChoose: ";

            int c;
            std::cin >> c;

            if (c == 3) break;

            std::string u, p;
            std::cout << "Username: ";
            std::cin >> u;

            std::cout << "Password: ";
            p = getPassword();

            std::string cmd = (c == 1 ? "login " : "signup ") + u + " " + p + "\n";

            send(sock, cmd.c_str(), cmd.size(), 0);

            std::string res = recvAll(sock);

            std::cout << "Server: " << res;

            if (res.find("LOGIN_OK") != std::string::npos)
                loggedIn = true;
        }
        else {
            std::cout << "\n=== MUSIC MENU ===\n";
            std::cout << "1. List songs\n";
            std::cout << "2. Play song\n";
            std::cout << "3. Logout\n";
            std::cout << "Choose: ";

            int c;
            std::cin >> c;

            if (c == 3) {
                loggedIn = false;
                continue;
            }

            if (c == 1) {
                std::string cmd = "list\n";
                send(sock, cmd.c_str(), cmd.size(), 0);

                std::cout << recvAll(sock);
            }

            else if (c == 2) {
                std::string song;
                std::cout << "Enter song name: ";
                std::cin >> song;

                std::string cmd = "play " + song + "\n";
                send(sock, cmd.c_str(), cmd.size(), 0);

                std::ofstream file("temp.mp3", std::ios::binary);

                char buffer[1024];
                int bytes;

                while ((bytes = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
                    file.write(buffer, bytes);

                    if (bytes < 1024)
                        break;
                }

                file.close();

                system("start temp.mp3"); // Windows
            }
        }
    }

    close(sock);
    return 0;
}