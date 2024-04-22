#include "commands.hpp"

#include <cstddef>
#include <string>

#include "Channel.hpp"

void is_command(std::string &buf, Server &server, Client &client) {
  std::string msg, command, channel;
  size_t index = buf.find('\n', 0);
  t_func func[1];

  func[0].command = "JOIN";
  func[0].action = &join;
  if (index == std::string::npos) {
    return;
  }
  msg = buf.substr(0, index);
  index = msg.find(" ");
  if (index == std::string::npos) return;
  command = buf.substr(0, index);
  for (int i = 0; i < 1; i++) {
    if (command == func[i].command) func[i].action(server, client, msg);
  }
}

void join(Server &server, Client &client, std::string &msg) {
  size_t begin, end;
  std::string channel;

  // Parse for join function ==> /join #<channel>
  begin = msg.find('#');
  if (begin == std::string::npos) return;
  begin++;
  end = msg.find(' ', begin);
  channel = msg.substr(begin, (end - begin));

  // Actual join function
  if (server.AddChannel(channel, client))
    // Existe channel mandar mensagem
    return;
  // Bem sucedido criar um novo registo de user como admin
}
