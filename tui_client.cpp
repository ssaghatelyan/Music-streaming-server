#include <ncurses.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define IP   "127.0.0.1"

int g_sock;

void net_send(const std::string& line) {
    std::string s = line + "\n";
    send(g_sock, s.c_str(), s.size(), 0);
}

std::string net_recv_string() {
    size_t len = 0;
    recv(g_sock, &len, sizeof(len), MSG_WAITALL);
    std::string s(len, '\0');
    recv(g_sock, &s[0], len, MSG_WAITALL);
    return s;
}

void print_center(int y, const std::string& s) {
    mvprintw(y, (COLS - s.size()) / 2, "%s", s.c_str());
}

std::string read_line(int y, int x, bool secret = false) {
    echo(); curs_set(1);
    if (secret) noecho();
    char buf[128] = {};
    move(y, x); getnstr(buf, 127);
    noecho(); curs_set(0);
    return buf;
}

void status(const std::string& msg, int color) {
    attron(COLOR_PAIR(color) | A_BOLD);
    mvprintw(LINES - 2, 2, "%-*s", COLS - 4, msg.c_str());
    attroff(COLOR_PAIR(color) | A_BOLD);
    refresh();
}

std::string screen_auth() {
    clear(); box(stdscr, 0, 0);
    attron(A_BOLD | COLOR_PAIR(2));
    print_center(2, "=== MUSIC CLIENT ===");
    attroff(A_BOLD | COLOR_PAIR(2));
    print_center(5, "[1] Login");
    print_center(6, "[2] Sign Up");
    print_center(7, "[q] Quit");
    refresh();

    int ch = getch();
    if (ch == 'q') return "\x01";
    if (ch != '1' && ch != '2') return "";
    bool is_login = (ch == '1');

    clear(); box(stdscr, 0, 0);
    attron(A_BOLD);
    print_center(2, is_login ? "-- LOGIN --" : "-- SIGN UP --");
    attroff(A_BOLD);

    mvprintw(5, 4, "Username: ");
    std::string user = read_line(5, 14);

    mvprintw(7, 4, "Password: ");
    std::string pass = read_line(7, 14, true);

    status("Connecting...", 3);
    net_send((is_login ? "login " : "signup ") + user + " " + pass);
    std::string res = net_recv_string();

    if (res == "LOGIN_OK")  { status("Welcome, " + user + "!", 1); napms(700); return user; }
    if (res == "SIGNUP_OK") { status("Account created! Now log in.", 1); napms(1000); return ""; }
    status(res.empty() ? "Server error" : res, 4);
    napms(1200);
    return "";
}

bool download_song(const std::string& song, const std::string& path) {
    net_send("play " + song);
    size_t file_size = 0;
    recv(g_sock, &file_size, sizeof(file_size), MSG_WAITALL);
    if (file_size == 0) return false;

    std::ofstream f(path, std::ios::binary);
    char buf[4096];
    size_t got = 0;
    while (got < file_size) {
        int n = recv(g_sock, buf, std::min(sizeof(buf), file_size - got), 0);
        if (n <= 0) break;
        f.write(buf, n);
        got += n;
        int bar = got * (COLS - 10) / file_size;
        mvprintw(LINES/2+2, 4, "[%-*s] %3d%%",
                 COLS-10, std::string(bar,'#').c_str(), (int)(got*100/file_size));
        refresh();
    }
    return got >= file_size;
}

void screen_main(const std::string& username) {
    std::vector<std::string> playlist;

    while (true) {
        clear(); box(stdscr, 0, 0);
        attron(A_BOLD | COLOR_PAIR(2));
        print_center(1, "=== MUSIC CLIENT ===");
        attroff(A_BOLD | COLOR_PAIR(2));
        attron(COLOR_PAIR(1));
        print_center(2, "User: " + username);
        attroff(COLOR_PAIR(1));

        mvprintw(5, 4, "[1] List songs");
        mvprintw(6, 4, "[2] Play  (stream to /tmp/)");
        mvprintw(7, 4, "[3] Download (save to ~/Downloads/)");
        mvprintw(8, 4, "[4] Logout");
        mvprintw(9, 4, "[q] Quit");

        if (!playlist.empty()) {
            attron(COLOR_PAIR(3));
            mvprintw(11, 4, "Playlist: %d tracks cached", (int)playlist.size());
            attroff(COLOR_PAIR(3));
        }
        refresh();

        int ch = getch();

        if (ch == '1') {
            net_send("list");
            std::string raw = net_recv_string();
            playlist.clear();
            std::istringstream ss(raw); std::string line;
            while (std::getline(ss, line)) if (!line.empty()) playlist.push_back(line);

            clear(); box(stdscr, 0, 0);
            attron(A_BOLD); print_center(1, "PLAYLIST"); attroff(A_BOLD);
            for (int i = 0; i < (int)playlist.size() && i < LINES-5; i++)
                mvprintw(3+i, 4, "%2d. %s", i+1, playlist[i].c_str());
            mvprintw(LINES-2, 4, "Press any key...");
            refresh(); getch();
        }

        else if (ch == '2' || ch == '3') {
            if (playlist.empty()) { status("Run [1] List first!", 4); napms(1000); continue; }

            clear(); box(stdscr, 0, 0);
            attron(A_BOLD); print_center(1, "SELECT TRACK"); attroff(A_BOLD);
            for (int i = 0; i < (int)playlist.size() && i < LINES-5; i++)
                mvprintw(3+i, 4, "%2d. %s", i+1, playlist[i].c_str());
            mvprintw(LINES-2, 4, "Number: ");
            refresh();

            echo(); curs_set(1);
            int n; scanw("%d", &n);
            noecho(); curs_set(0);

            if (n < 1 || n > (int)playlist.size()) { status("Invalid number", 4); napms(800); continue; }
            std::string song = playlist[n-1];

            std::string out_path;
            if (ch == '2') {
                out_path = "/tmp/" + song;
            } else {
                const char* home = getenv("HOME");
                out_path = std::string(home ? home : "/tmp") + "/Downloads/" + song;
            }

            clear(); box(stdscr, 0, 0);
            print_center(LINES/2,   song);
            print_center(LINES/2+1, "Downloading...");
            refresh();

            bool ok = download_song(song, out_path);
            if (ok && ch == '2') {
                endwin();
                system(("mpg123 \"" + out_path + "\"").c_str());
                initscr(); cbreak(); noecho(); curs_set(0);
            }
            status(ok ? "Done! Saved to: " + out_path : "Error: file not found", ok ? 1 : 4);
            napms(1500);
        }

        else if (ch == '4') return;
        else if (ch == 'q') { endwin(); exit(0); }
    }
}

int main() {
    g_sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);

    if (connect(g_sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Cannot connect to %s:%d\n", IP, PORT);
        return 1;
    }

    initscr(); cbreak(); noecho(); curs_set(0);
    start_color(); use_default_colors();
    init_pair(1, COLOR_GREEN,  -1);
    init_pair(2, COLOR_YELLOW, -1);
    init_pair(3, COLOR_CYAN,   -1);
    init_pair(4, COLOR_RED,    -1);

    while (true) {
        std::string user = screen_auth();
        if (user == "\x01") break;
        if (user.empty())   continue;
        screen_main(user);
    }

    endwin();
    close(g_sock);
    return 0;
}
