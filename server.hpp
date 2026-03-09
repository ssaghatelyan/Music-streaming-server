#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <cstring>
#include <fstream>

#define PORT 1234
#define BUFFER 4096

bool login(std::string user, std::string pass)
{
    std::ifstream file("users.txt");
    std::string u, p;

    while (file >> u >> p)
    {
        if (u == user && p == pass)
            return true;
    }
    return false;
}

bool signup(std::string user, std::string pass)
{
    std::ofstream file("users.txt", std::ios::app);
    file << user << " " << pass << std::endl;
    return true;
}

void play_file(int client)
{
    int fd = open("song.mp3", O_RDONLY);
    char buffer[BUFFER];
    int bytes;

    while ((bytes = read(fd, buffer, BUFFER)) > 0)
    {
        send(client, buffer, bytes, 0);
    }

    close(fd);
}

void download_file(int client)
{
    int fd = open("song.mp3", O_RDONLY);

    struct stat st;
    fstat(fd, &st);

    off_t offset = 0;
    sendfile(client, fd, &offset, st.st_size);

    close(fd);
}

int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);

    std::cout << "Server started\n";

    while (true)
    {
        int client = accept(server_fd, nullptr, nullptr);

        char buffer[BUFFER] = {0};
        read(client, buffer, BUFFER);

        std::string command(buffer);

        if (command.find("SIGNUP") != std::string::npos)
        {
            std::string user, pass;
            read(client, buffer, BUFFER);
            sscanf(buffer, "%s %s", &user[0], &pass[0]);

            signup(user, pass);
            send(client, "Signup OK", 9, 0);
        }

        if (command.find("LOGIN") != std::string::npos)
        {
            char user[50], pass[50];
            read(client, buffer, BUFFER);
            sscanf(buffer, "%s %s", user, pass);

            if (login(user, pass))
            {
                send(client, "LOGIN_OK", 8, 0);

                read(client, buffer, BUFFER);

                if (strncmp(buffer, "PLAY", 4) == 0)
                    play_file(client);
                else if (strncmp(buffer, "DOWNLOAD", 8) == 0)
                    download_file(client);
            }
            else
            {
                send(client, "LOGIN_FAIL", 10, 0);
            }
        }

        close(client);
    }
}
