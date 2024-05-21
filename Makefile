FILENAME =  Client/Client.cpp \
			Server/Server.cpp \
			Channel/Channel.cpp \
			main.cpp

SRCS_DIR = srcs

SRCS = $(addprefix $(SRCS_DIR)/, $(FILENAME))

OBJS_DIR = objs

OBJS = $(subst $(SRCS_DIR)/, $(OBJS_DIR)/, $(SRCS:.cpp=.o))

DEPS = $(OBJS:.o=.d)

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I includes -g #-fsanitize=address

NAME = ircserv

RM = rm -rf

all:	$(NAME)

$(NAME):	$(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

-include $(DEPS)

$(OBJS_DIR)/%.o:	$(SRCS_DIR)/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

clean:
	$(RM) objs/

fclean:	clean
	$(RM) $(NAME)

t:	re
	clear
	./$(NAME)

re: fclean all
