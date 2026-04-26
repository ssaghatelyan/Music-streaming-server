#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <limits>
#include <vector>
#include <sstream>

#define PORT 8080

int readInt() {
    int c;
    while (!(std::cin >> c)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return c;
}

int recvAll(int sock, void* buffer, size_t size) {
    size_t total = 0;
    char* buf = (char*)buffer;

    while (total < size) {
        int bytes = recv(sock, buf + total, size - total, 0);
        if (bytes <= 0)
            return -1;
        total += bytes;
    }

    return total;
}

std::string recvString(int sock) {
    size_t len;

    if (recvAll(sock, &len, sizeof(len)) <= 0)
        return "";

    std::string data(len, '\0');

    if (recvAll(sock, &data[0], len) <= 0)
        return "";

    return data;
}

std::string getPassword() {
    std::string pass;

    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);

    newt = oldt;
    newt.c_lflag &= ~ECHO;

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::getline(std::cin, pass);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl;

    return pass;
}

void playSong(const std::string& filepath) {
    std::string cmd = "cmd.exe /c start \"\" \"$(wslpath -w " + filepath + ")\" 2>/dev/null";
    if (system(cmd.c_str()) != 0) {
        std::string winPath;
        FILE* pipe = popen(("wslpath -w " + filepath).c_str(), "r");
        if (pipe) {
            char buf[512];
            if (fgets(buf, sizeof(buf), pipe)) {
                winPath = buf;
                if (!winPath.empty() && winPath.back() == '\n')
                    winPath.pop_back();
            }
            pclose(pipe);
        }
        if (!winPath.empty()) {
            system(("explorer.exe \"" + winPath + "\"").c_str());
        } else {
            std::cout << "File saved to: " << filepath << "\n";
            std::cout << "Open it manually in Windows Explorer.\n";
        }
    }
}

std::vector<std::string> parsePlaylist(const std::string& raw) {
    std::vector<std::string> songs;
    std::istringstream ss(raw);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (!line.empty())
            songs.push_back(line);
    }
    return songs;
}

void printPlaylist(const std::vector<std::string>& songs) {
    std::cout << "\n=== Playlist ===\n";
    for (size_t i = 0; i < songs.size(); i++)
        std::cout << i + 1 << ". " << songs[i] << "\n";
    std::cout << "================\n\n";
}

std::string selectSong(const std::vector<std::string>& songs) {
    if (songs.empty()) {
        std::cout << "Playlist is empty. Use '1 List' first.\n";
        return "";
    }
    printPlaylist(songs);
    std::cout << "Number: ";
    int n = readInt();
    if (n < 1 || n > (int)songs.size()) {
        std::cout << "Invalid number\n";
        return "";
    }
    return songs[n - 1];
}

std::string getDownloadDir() {
    FILE* pipe = popen("cmd.exe /c \"echo %USERPROFILE%\" 2>/dev/null", "r");
    if (pipe) {
        char buf[512];
        if (fgets(buf, sizeof(buf), pipe)) {
            std::string winProfile = buf;
            while (!winProfile.empty() && (winProfile.back() == '\n' || winProfile.back() == '\r'))
                winProfile.pop_back();
            if (winProfile.size() > 2 && winProfile[1] == ':') {
                char drive = tolower(winProfile[0]);
                std::string rest = winProfile.substr(2);
                for (char& ch : rest)
                    if (ch == '\\') ch = '/';
                pclose(pipe);
                return std::string("/mnt/") + drive + rest + "/Downloads";
            }
        }
        pclose(pipe);
    }
    return std::string(getenv("HOME") ? getenv("HOME") : "/tmp");
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cout << "Connection error\n";
        return 1;
    }

    bool logged = false;
    std::vector<std::string> playlist;


    while (true) {
        if (!logged) {
            std::cout << "1 Login\n2 Signup\n3 Exit\n";

            int c = readInt();

            if (c == 3)
                break;

            std::string u, p;

            std::cout << "User: ";
            std::getline(std::cin, u);

            std::cout << "Pass: ";
            p = getPassword();

            std::string msg = (c == 1 ? "login " : "signup ") + u + " " + p + "\n";
            send(sock, msg.c_str(), msg.size(), 0);

            std::string res = recvString(sock);

            if (res.empty()) {
                std::cout << "Server error\n";
                break;
            }

            std::cout << res << "\n";

            if (res == "LOGIN_OK") {
                logged = true;
                playlist.clear();
            }
        }
        else {
            std::cout << "1 List\n2 Play\n3 Download\n4 Logout\n";

            int c = readInt();

            if (c == 4) {
                logged = false;
                playlist.clear();
                continue;
            }

            auto receiveSong = [&](const std::string& song, const std::string& outPath) -> bool {
                std::string msg = "play " + song + "\n";
                send(sock, msg.c_str(), msg.size(), 0);

                size_t file_size = 0;
                if (recvAll(sock, &file_size, sizeof(file_size)) <= 0) {
                    std::cout << "Error receiving file size\n";
                    return false;
                }

                if (file_size == 0) {
                    std::cout << "File not found or access denied\n";
                    return false;
                }

                std::ofstream file(outPath, std::ios::binary);
                if (!file.is_open()) {
                    std::perror("Cannot open output file");
                    return false;
                }

                char buffer[4096];
                size_t received = 0;

                while (received < file_size) {
                    size_t toRead = std::min(sizeof(buffer), file_size - received);
                    int bytes = recv(sock, buffer, toRead, 0);
                    if (bytes <= 0) break;
                    file.write(buffer, bytes);
                    received += bytes;
                }

                file.close();

                if (received < file_size) {
                    std::cout << "Warning: file received partially (" << received << "/" << file_size << " bytes)\n";
                    return false;
                }

                return true;
            };

            if (c == 1) {
                std::string msg = "list\n";
                send(sock, msg.c_str(), msg.size(), 0);

                std::string raw = recvString(sock);
                if (raw.empty()) {
                    std::cout << "Server error\n";
                    continue;
                }

                playlist = parsePlaylist(raw);
                printPlaylist(playlist);
            }

            if (c == 2) {
                std::string song = selectSong(playlist);
                if (song.empty()) continue;

                std::string outPath = "/tmp/song.mp3";
                if (receiveSong(song, outPath)) {
                    std::cout << "Playing: " << song << "\n";
                    playSong(outPath);
                }
            }

            if (c == 3) {
                std::string song = selectSong(playlist);
                if (song.empty()) continue;

                std::string downloadDir = getDownloadDir();
                std::string outPath = downloadDir + "/" + song;
                std::cout << "Saving to: " << outPath << "\n";

                if (receiveSong(song, outPath)) {
                    std::cout << "Downloaded: " << song << "\n";
                }
            }
        }
    }

    close(sock);
    return 0;
}