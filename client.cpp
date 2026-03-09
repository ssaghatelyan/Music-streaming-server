#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstring>

#define PORT 1234
#define BUFFER 4096

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);

    connect(sock, (sockaddr*)&serv, sizeof(serv));

    std::cout << "1 LOGIN\n2 SIGNUP\n";
    int choice;
    std::cin >> choice;

    char user[50], pass[50];

    std::cout << "Username: ";
    std::cin >> user;

    std::cout << "Password: ";
    std::cin >> pass;

    char buffer[BUFFER];

    if (choice == 2)
    {
        send(sock, "SIGNUP", 6, 0);

        sprintf(buffer, "%s %s", user, pass);
        send(sock, buffer, strlen(buffer), 0);

        recv(sock, buffer, BUFFER, 0);
        std::cout << buffer << std::endl;
        return 0;
    }

    send(sock, "LOGIN", 5, 0);

    sprintf(buffer, "%s %s", user, pass);
    send(sock, buffer, strlen(buffer), 0);

    recv(sock, buffer, BUFFER, 0);

    if (strncmp(buffer, "LOGIN_OK", 8) != 0)
    {
        std::cout << "Login failed\n";
        return 0;
    }

    std::cout << "1 PLAY\n2 DOWNLOAD\n";
    std::cin >> choice;

    if (choice == 1)
        send(sock, "PLAY", 4, 0);
    else
        send(sock, "DOWNLOAD", 8, 0);

    int bytes;

    if (choice == 1)
    {
        FILE* player = popen("mpg123 -", "w");

        while ((bytes = recv(sock, buffer, BUFFER, 0)) > 0)
        {
            fwrite(buffer, 1, bytes, player);
        }

        pclose(player);
    }
    else
    {
        int file = open("song.mp3",
                        O_WRONLY | O_CREAT | O_TRUNC, 0644);

        while ((bytes = recv(sock, buffer, BUFFER, 0)) > 0)
        {
            write(file, buffer, bytes);
        }

        close(file);
        std::cout << "Downloaded\n";
    }

    close(sock);
}
