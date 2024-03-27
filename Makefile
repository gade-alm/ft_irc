SRCS		= srcs/Server.cpp \
				main.cpp \

OBJS		= $(SRCS:.cpp=.o)

CXX			= c++

CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 

NAME		= ircserv

all: $(NAME)

$(NAME): $(OBJS) 
		$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME) -fsanitize=address -g

RM		= rm -rf

t: re
	clear
	./$(NAME)

clean: 
	$(RM) $(OBJS)

fclean:
	$(RM) $(OBJS) $(NAME)

re:fclean all