SERVER  = server
CLIENT  = client
TUI     = tui_client

GUI_DIR = gui
GUI_BIN = $(GUI_DIR)/build/client

CXX      = g++
CXXFLAGS = -Wall -Wextra -Werror
LDFLAGS  = -pthread

SRC_SERVER = server.cpp
SRC_CLIENT = client.cpp
SRC_TUI    = tui_client.cpp

OBJ_SERVER = $(SRC_SERVER:.cpp=.o)
OBJ_CLIENT = $(SRC_CLIENT:.cpp=.o)
OBJ_TUI    = $(SRC_TUI:.cpp=.o)

all: $(SERVER) $(CLIENT) $(TUI)

$(SERVER): $(OBJ_SERVER)
	$(CXX) $(CXXFLAGS) $(OBJ_SERVER) -o $(SERVER) $(LDFLAGS)

$(CLIENT): $(OBJ_CLIENT)
	$(CXX) $(CXXFLAGS) $(OBJ_CLIENT) -o $(CLIENT)

$(TUI): $(OBJ_TUI)
	$(CXX) $(CXXFLAGS) $(OBJ_TUI) -o $(TUI) -lncurses

gui:
	cmake -B $(GUI_DIR)/build -S $(GUI_DIR) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(GUI_DIR)/build --parallel

clean:
	rm -f $(OBJ_SERVER) $(OBJ_CLIENT) $(OBJ_TUI)
	rm -rf $(GUI_DIR)/build

fclean: clean
	rm -f $(SERVER) $(CLIENT) $(TUI)

re: fclean all

.PHONY: all gui clean fclean re