SERVER  = server
CLIENT  = client
TUI     = tui_client
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

clean:
	rm -f $(OBJ_SERVER) $(OBJ_CLIENT) $(OBJ_TUI)

fclean: clean
	rm -f $(SERVER) $(CLIENT) $(TUI)

re: fclean all
.PHONY: all clean fclean re