#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>

#include "Channel.hpp"
#include "Client.hpp"
#include "Server.hpp"

typedef struct s_commands {
  std::string command;

  void (*action)(Server &server, Client &client, std::string &msg);
} t_func;

void is_command(std::string &buf, Server &server, Client &client);
void join(Server &server, Client &client, std::string &msg);

#endif  // !COMMANDS_HPP
