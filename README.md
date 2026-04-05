# Music Streaming Server

A client-server application for streaming and downloading music over a local network. Written in C++ using TCP sockets.

## Features

- User registration and login with password hashing (FNV-1a)
- Browse the list of tracks on the server
- Stream a track to `/tmp/` and play it with `mpg123`
- Download a track to `~/Downloads/`
- Multi-threaded server — each client runs in its own thread
- Protection against path traversal attacks
- TUI client built with `ncurses` — a clean interface right in the terminal

## Structure

```
Music-streaming-server/
├── server.cpp        # TCP server, auth, file serving
├── client.cpp        # Simple client (no UI)
├── tui_client.cpp    # Client with ncurses TUI
├── Makefile
├── songs/            # Folder with mp3 files (create manually)
└── users.txt         # User database (created automatically)
```

## Dependencies

```bash
sudo apt install libncurses-dev mpg123
```

## Build

```bash
make          # build everything
make re       # clean rebuild
make clean    # remove .o files
make fclean   # remove all build artifacts
```

## Usage

**Terminal 1 — server:**
```bash
./server
```

**Terminal 2 — TUI client:**
```bash
./tui_client
```

Or the plain client without UI:
```bash
./client
```

## TUI Controls

| Key | Action |
|-----|--------|
| `1` | Fetch track list from server |
| `2` | Stream a track (downloads to `/tmp/` and plays) |
| `3` | Download a track to `~/Downloads/` |
| `4` | Logout |
| `q` | Quit |

## Adding Music

Put `.mp3` files into the `songs/` folder next to the server binary:

```bash
mkdir -p songs
cp ~/Music/*.mp3 songs/
```

## Protocol

Client and server communicate over TCP on port `8080`. Commands:

```
login <user> <pass>    # log in
signup <user> <pass>   # register
list                   # get file list
play <filename>        # receive a file
```

Strings are sent with a `size_t` length prefix. Files are sent as raw bytes preceded by their size.

## Requirements

- Linux (`sendfile`, `dirent`, `pthreads`)
- g++ with C++17 support
- `libncurses-dev` for the TUI client
- `mpg123` for playback in the TUI client