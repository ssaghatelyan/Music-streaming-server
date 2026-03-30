NAME = server
CLIENT = client

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror

SRC_SERVER = server.cpp auth.cpp utils.cpp
SRC_CLIENT = client.cpp utils.cpp

OBJ_SERVER = $(SRC_SERVER:.cpp=.o)
OBJ_CLIENT = $(SRC_CLIENT:.cpp=.o)

all: $(NAME) $(CLIENT)

$(NAME): $(OBJ_SERVER)
	$(CXX) $(CXXFLAGS) $(OBJ_SERVER) -o $(NAME) -lssl -lcrypto

$(CLIENT): $(OBJ_CLIENT)
	$(CXX) $(CXXFLAGS) $(OBJ_CLIENT) -o $(CLIENT) -lssl -lcrypto

clean:
	rm -f $(OBJ_SERVER) $(OBJ_CLIENT)

fclean: clean
	rm -f $(NAME) $(CLIENT)

re: fclean all