#include "Server.hpp"

#include <algorithm>
#include <limits>
#include <sstream>
#include <string>

static void checkPassword(std::string password);

Server::Server(void) {}

Server::Server(const Server &) {}

Server::~Server(void) {}

Server::Server(const char *portValue, const std::string &passwordValue) {
  _clientfd = 0;
  maxfds = 0;
  int used = 1;
  if (strlen(portValue) > 6) {
    std::cerr << "Wrong input on port" << std::endl;
    exit (1);
  }
  int port = atoi(portValue);
  if (port < MINPORT || port > MAXPORT) {
    std::cout << ("Wrong number on port") << std::endl;
    exit(1);
  }
  _serverPort = port;
  _password = passwordValue;
  checkPassword(passwordValue);
  initAddr();
  setSocket(socket(serverAddr.sin_family, SOCK_STREAM, PROTOCOL));
  if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &used,
                 sizeof(used)) == -1) {
    close(_serverSocket);
    // perror("setsockopt");
    return;
  }
  setBind();
  listenSockets();
}

void Server::setSocket(int socketFD) {
  _serverSocket = socketFD;
  if (_serverSocket == -1) {
    // perror("setSocket");
    return;
  }
}

void Server::setBind(void) {
  _bindSocket = bind(_serverSocket, (struct sockaddr *)&serverAddr,
                     sizeof(struct sockaddr));
  if (_bindSocket == -1) {
    close(_serverSocket); 
    // perror("setBind");
    exit(1);
  }
}

void Server::initAddr(void) {
  memset(&(serverAddr.sin_zero), 0, sizeof(serverAddr.sin_zero));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(_serverPort);
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  if (serverAddr.sin_addr.s_addr != 0) {
    // perror("initAddr");
    return;
  }
}

void Server::listenSockets(void) {
  _listenSocket = listen(_serverSocket, BACKLOG);
  if (_listenSocket == -1) {
    // perror("Error while opening sockets");
    return;
  }
}

int Server::getSocket(void) { return _serverSocket; }

void Server::prepareFDs(void) {
  FD_ZERO(&_selectfds);
  FD_ZERO(&_masterfds);

  FD_SET(_serverSocket, &_masterfds);
  maxfds = _serverSocket;
}

void Server::selectLoop(struct sockaddr_in _clientaddr) {
  _selectfds = _masterfds;
  if (select(maxfds + 1, &_selectfds, NULL, NULL, NULL) == -1) {
    // perror("SELECT:");
    return;
  }
  for (int i = 0; i <= maxfds; i++) {
    if (FD_ISSET(i, &_selectfds)) {
      if (i == _serverSocket) {
        socklen_t addrSize = sizeof(_clientaddr);
        if ((_clientfd = accept(_serverSocket, (struct sockaddr *)&_clientaddr,
                                &addrSize)) != -1) {
          FD_SET(_clientfd, &_masterfds);
          if (_clientfd > maxfds) maxfds = _clientfd;
          Client client(_clientfd);
          _Clients.push_back(client);
        } else
          perror("accept error");
      } else {
        int numbytes = 0;
        if ((numbytes = recv(i, buf, sizeof(buf), MSG_DONTWAIT)) < 1) {
          if (errno == EAGAIN || errno == EWOULDBLOCK)
            continue;
          else {
            if (searchClient(i) != _Clients.end()) {
              disconnectClient(searchClient(i));
              return;
            }
            // std::cout << " CAIU NESATA MERDA " << std::endl;
            // close(i);
            // FD_CLR(i, &_masterfds);
          }
        } else {
          std::string buffer(buf, numbytes);
          std::vector<Client>::iterator it = searchClient(i);
          if (it != _Clients.end()) {
            Client &client = *it;
            client.authenticateClient(_password, buf, _Clients);
            std::cout << "CLIENTFD: " << client.getFD()
                      << " is Auth: " << client.getAuth() << std::endl;
            if (client.getAuth()) cmdHandler(buffer, client);
          } else {
            disconnectClient(it);
            return;
          }
        }
        memset(buf, 0, 1024);
      }
    }
  }
}

std::vector<Client>::iterator Server::searchClient(int fd) {
  std::vector<Client>::iterator it;

  for (it = _Clients.begin(); it != _Clients.end(); ++it) {
    if (it->getFD() == fd) break;
  }
  return it;
}

std::vector<Client>::iterator Server::searchClient(std::string name) {
  std::vector<Client>::iterator it;

  for (it = _Clients.begin(); it != _Clients.end(); ++it) {
    if (it->getNick() == name) break;
  }
  return it;
}

std::vector<Channel>::iterator Server::searchChannel(std::string channelname) {
  std::vector<Channel>::iterator it;
  if (channelname[0] != '#') channelname = "#" + channelname;
  for (it = _Channels.begin(); it != _Channels.end(); ++it) {
    if (it->getName() == channelname) {
      break;
    }
  }
  return it;
}

void Server::disconnectClient(std::vector<Client>::iterator it) {
  Client &client = *it;
  std::string message = "You have been disconnected.";
  sendMessage(message, client.getFD());
  outOfChannels(client);
  close(client.getFD());
  FD_CLR(client.getFD(), &_masterfds);
  _Clients.erase(it);
}

void Server::cmdHandler(std::string buffer, Client &client) {
  // std::cout << buffer << std::endl;
  std::vector<std::string> CMD = parseCMD(buffer);
  void (Server::*myCMDS[8])(std::vector<std::string>, Client &) = {
      &Server::joinChannel,  &Server::quitServer,
      &Server::deliveryMSG,  &Server::kickFromChannel,
      &Server::topicChannel, &Server::invite,
      &Server::mode,         &Server::part};
  long unsigned int index;
  std::string cmds[8] = {"JOIN",  "QUIT",   "PRIVMSG", "KICK",
                         "TOPIC", "INVITE", "MODE",    "PART"};

  for (index = 0; index < sizeof(cmds) / sizeof(cmds[0]); index++) {
    if (!CMD.empty() && CMD[0] == cmds[index]) break;
  }
  if (index < sizeof(cmds) / sizeof(cmds[0]))
    (this->*myCMDS[index])(CMD, client);
}

void Server::joinChannel(std::vector<std::string> CMD, Client &client) {
  if (CMD.size() < 2) return;
  std::string channelname = CMD[1];
  if (channelname[0] != '#') channelname = "#" + channelname;
  if (!channelPrep(channelname, client, CMD)) {
    // INVALID MODE
    return;
  }
  std::string output =
      ":" + client.getNick() + "!" + client.getUser() + " JOIN " + channelname;
  sendMessage(output, client.getFD());

  std::string message = ":" + client.getNick() + "!" + client.getUser() +
                        " JOIN " + channelname + "\r\n";
  std::vector<Channel>::iterator itChannel = searchChannel(channelname);
  for (std::vector<Client>::iterator itClient = itChannel->beginUsers();
       itClient != itChannel->endUsers(); itClient++) {
    if (itClient->getFD() != client.getFD())
      sendMessage(message, itClient->getFD());
  }
  /* 	std::vector<Channel>::iterator itChannel = searchChannel(channelname);
          itChannel->printUsers(); */
}

void Server::quitServer(std::vector<std::string> CMD, Client &client) {
  (void)CMD;
  std::vector<Client>::iterator it = searchClient(client.getFD());
  // Missing Channels disconnect.
  disconnectClient(it);
}

bool Server::channelPrep(std::string channelname, Client &client,
                         std::vector<std::string> CMD) {
  std::vector<Channel>::iterator itChannel = searchChannel(channelname);
  std::vector<Client>::iterator it;
  std::vector<int>::iterator itInv;

  if (itChannel != _Channels.end()) {
    it = itChannel->searchClient(client.getFD());
    if (it != itChannel->endUsers()) return false;
    // Caso esteja em Invite Mode e sem invitation retornar logo.
    itInv = std::find(itChannel->_invitation.begin(),
                      itChannel->_invitation.end(), client.getFD());
    // std::cout << "CLIENT FD: " << client.getFD() << " "
    //           << itChannel->_invitation[0] << std::endl;
    if (itChannel->getPassword() != "" && (CMD.size() < 3 || \
        CMD[2] != itChannel->getPassword())){
          sendMessage("Wrong password to join channel.", client.getFD());
          return false;
        } 
    if (itChannel->getLimitMode() &&
        itChannel->getLimit() == itChannel->getUserOn().size()) {
      sendMessage("Channel is full.", client.getFD());
      return false;
    }
    if (itChannel->getInvMode() && itInv == itChannel->_invitation.end()) {
      sendMessage("You are not invited to this channel.", client.getFD());
      return false;
    }
    if (itInv != itChannel->_invitation.end())
      itChannel->_invitation.erase(itInv);
    if (itChannel->searchClient(client.getFD()) == itChannel->endUsers())
      itChannel->addUser(client);
    return true;
    }
  Channel channel(channelname);
  channel.addUser(client);
  std::vector<Client>::iterator itClient = channel.searchClient(client.getFD());
  itClient->setOP(true);
  _Channels.push_back(channel);

  itChannel = searchChannel(channelname);
  // itChannel->printUsers();
  return true;
}

std::string Server::prepReason(std::vector<std::string> CMD, int i) {
  std::string reason;

  for (std::vector<std::string>::iterator it = CMD.begin() + i; it != CMD.end();
       it++) {
    if (*it != CMD[i]) reason += " ";
    reason += *it;
  }
  return reason;
}

void Server::deliveryMSG(std::vector<std::string> CMD, Client &client) {
  std::string channelname = CMD[1];
  std::string message = ":" + client.getNick() + "!~" + client.getUser() +
                        " PRIVMSG " + channelname + " " + prepReason(CMD, 2);

  if (channelname[0] != '#') {
    std::vector<Client>::iterator itClient = searchClient(channelname);
    if (itClient == _Clients.end()) return;
    sendMessage(message, itClient->getFD());
    return;
  }

  std::vector<Channel>::iterator itChannel = searchChannel(channelname);
  if (itChannel == _Channels.end()) return;
  if (!itChannel->clientIsHere(client.getFD())) return;
  for (std::vector<Client>::iterator itClient = itChannel->beginUsers();
       itClient != itChannel->endUsers(); itClient++) {
    if (itClient->getFD() != client.getFD())
      sendMessage(message, itClient->getFD());
  }
}

void Server::kickFromChannel(std::vector<std::string> CMD, Client &client) {
  // :server_name KICK #channelname username :reason
  std::string channelname;
  if (channelname[0] != '#') channelname = "#" + channelname;
  std::string nick;
  if (CMD.size() < 3) return;
  channelname = CMD[1];
  nick = CMD[2];
  std::string reason = (CMD.size() >= 4) ? prepReason(CMD, 3) : "";

  std::vector<Channel>::iterator it = searchChannel(channelname);
  if (it == _Channels.end()) return;
  if (it->searchClient(client.getNick()) == it->endUsers() || \
     !it->searchClient(client.getNick())->isOP()) {
    std::string msg = ":IRC PRIVMSG " + channelname +
                      " :You dont have the rights to kick Users. ";
    sendMessage(msg, client.getFD());
    return;
  }
  if (it->searchClient(nick) == it->endUsers()) {
    std::string msg = ":IRC PRIVMSG " + channelname + " :User not found.";
    sendMessage(msg, client.getFD());
    return;
  }
  std::string cmd = ":" + client.getNick() + "!" + client.getUser() + " KICK " +
                    channelname + " " + nick +
                    ((reason.empty()) ? "" : (" " + reason)) + "\r\n";
  for (std::vector<Client>::iterator itClient = it->beginUsers();
       itClient != it->endUsers(); itClient++) {
    sendMessage(cmd, itClient->getFD());
  }
  it->rmUser(*it->searchClient(nick));
  return;
}

std::vector<std::string> Server::parseCMD(std::string buffer) {
  size_t start = 0;
  size_t end = 0;
  std::string word;
  std::vector<std::string> CMD;

  while (start != buffer.size()) {
    end = buffer.find(" ", start);
    if (end == std::string::npos) {
      (end != buffer.find("\r", start)) ? end = buffer.find("\r", start)
                                        : end = buffer.find("\n", start);
      if (start == end) break;
      word = buffer.substr(start, end - start);
      CMD.push_back(word);
      break;
      ;
    }
    word = buffer.substr(start, end - start);
    CMD.push_back(word);
    start = end + 1;
  }
  return CMD;
}

void Server::topicChannel(std::vector<std::string> CMD, Client &client) {
  std::string channelName;
  if (CMD.size() < 2) return;
  channelName = CMD[1];
  std::vector<Channel>::iterator it = searchChannel(channelName);

  if (it == _Channels.end()) return ;
  if (it->searchClient(client.getFD()) == it->endUsers()) return ;
  if (CMD.size() == 2) {
    if (it != _Channels.end()) {
      std::string topic = it->getTopic();
      std::string msg = ((!topic.empty()) ? ":IRC 332 " : ":IRC 331 ") +
                        client.getNick() + " " + channelName + " :" +
                        ((!topic.empty()) ? topic : "No topic is set") + "\r\n";
      sendMessage(msg, client.getFD());
    }
    return;
  }

  if (it->getTopicMode() && !it->searchClient(client.getFD())->isOP()) {
    std::string msg = ":IRC 482 " + client.getNick() + " " + channelName +
                      " :You're not channel operator\r\n";
    sendMessage(msg, client.getFD());
    return;
  }

  //: Nick!User@host TOPIC #channelname :new topic\r\n
  std::string topic;
  for (std::vector<std::string>::iterator itC = CMD.begin() + 2;
       itC != CMD.end(); itC++) {
    if (*itC != CMD[2]) topic += " ";
    topic += *itC;
  }

  std::string msg = ":" + client.getNick() + "!" + client.getUser() +
                    " TOPIC " + channelName + " " + topic + "\r\n";
  // std::cout << "TOPIC: " << topic << std::endl;
  it->setTopic(topic.substr((topic[0] == ':') ? 1 : 0, topic.size() - ((topic[0] == ':') ? 1 : 0)));
  for (std::vector<Client>::iterator itClient = it->beginUsers();
       itClient != it->endUsers(); itClient++) {
    sendMessage(msg, itClient->getFD());
  }
}

void Server::part(std::vector<std::string> CMD, Client &client) {
  std::vector<Channel>::iterator channel;
  std::vector<Client>::iterator it;
  std::string msg;

  if (CMD.size() != 2) return;
  channel = searchChannel(CMD[1]);
  if (channel == _Channels.end()) return ;
  it = channel->searchClient(client.getFD());
  if (it == channel->endUsers()) return;
  channel->rmUser(client);
  msg = ':' + client.getNick() + '!' + client.getUser() + ' ' + CMD[0] + ' ' +
        CMD[1] ;
  sendMessage(msg, client.getFD());
}

// Invite Function
void Server::invite(std::vector<std::string> CMD, Client &client) {
  if (CMD.size() < 3) return;
  std::vector<Client>::iterator destiny = searchClient(CMD[1]);
  std::vector<Channel>::iterator channel = searchChannel(CMD[2]);
  std::vector<Client>::iterator verify;
  std::string msg;

  if (destiny == _Clients.end()) return;
  if (channel == _Channels.end()) return;
  verify = std::find(channel->beginUsers(), channel->endUsers(), client);
  if (verify == channel->endUsers()) return;
  if (!verify->isOP()) {
    std::string msg = ":IRC 482 " + client.getNick() + " " +
                      channel->getName() + " :You're not channel operator\r\n";
    return;
  }
  verify = channel->searchClient(CMD[1]);
  if (CMD[1] == client.getNick() || verify != channel->endUsers()) return;
  channel->_invitation.push_back(destiny->getFD());

  msg = ":" + client.getNick() + "!~" + client.getUser() + " INVITE " + destiny->getNick() + " :" + CMD[2];
  sendMessage(msg, destiny->getFD());

  msg = ":IRC 341 " + client.getNick() + " " + destiny->getNick() + " " + CMD[2];
  sendMessage(msg, client.getFD());
}

/*********************************************
 * Mode function has 5 flags                 *
 *  i  Sets invite only channel flag         *
 *  t Change or view Topics                  *
 *  k Set / Removes the channel key          *
 *  o Give / Take channel operator privilege *
 *  l Set / Romove the user limit to channel *
 *                                           *
 * To kknow if a flag is used must have a    *
 * + or - sign in a flag                     *
 ********************************************/

void Server::mode(std::vector<std::string> CMD, Client &client) {
  if (CMD.size() < 2) return;
  if (CMD.size() == 2) {
    printArgs(CMD, client);
    return;
  }
  size_t i = 0;
  char flag;
  for (; i < CMD.size(); i++){
    if (CMD[i][0] == '-' || CMD[i][0] == '+')
    {
      flag = CMD[i][0];
      break ;
    }
  }
  for (size_t j = 1; j < CMD[i].size(); j++) {
    if (CMD[i][j])
      switch (CMD[i][j])
      {
        case 'i':
          inviteOnly(CMD, client, flag);
          break;
        case 'o':
          operatorFlag(CMD, client, flag, i);
          break;
        case 't':
          topicFlag(CMD, client, flag);
          break;
        case 'k':
          passwordFlag(CMD, client, flag, i);
          break;
        case 'l':
          userLimitFlag(CMD, client, flag, i);
          break;
      }   
    }

  }

void Server::inviteOnly(std::vector<std::string> CMD, Client &client, char flag) {
  std::string channel, msg;
  std::vector<Channel>::iterator itChannel;
  std::vector<Client>::iterator itClient;
  bool mode;

  // if (CMD.size() != 3 ) return;
  channel = CMD[1];
  itChannel = searchChannel(channel);
  if (itChannel == _Channels.end()) return;
  itClient = itChannel->searchClient(client.getFD());
  if (itClient == itChannel->endUsers()) return;
  (flag == '+') ? mode = true : mode = false;
  if (!itClient->isOP() || itChannel->getInvMode() == mode) return;
  itChannel->setInvMode(mode);
  msg = (":" + client.getNick() + "!" + client.getUser() + " " + "MODE " + channel + " " + flag + 'i');
  sendToAll(msg, itChannel);
  //std::cout << "INVITE ONLY: " << itChannel->getInvMode() << std::endl;
}

void Server::topicFlag(std::vector<std::string> CMD, Client &client, char flag) {
  std::string channel, msg;
  std::vector<Channel>::iterator itChannel;
  std::vector<Client>::iterator itClient;
  bool mode;

  // if (CMD.size() != 3 ) return;
  channel = CMD[1];
  itChannel = searchChannel(channel);
  if (itChannel == _Channels.end()) return;
  itClient = itChannel->searchClient(client.getFD());
  if (itClient == itChannel->endUsers()) return;
  (flag == '+') ? mode = true : mode = false;
  if (!itClient->isOP() || itChannel->getTopicMode() == mode) return;
  itChannel->setTopicMode(mode);
  msg = (":" + client.getNick() + "!" + client.getUser() + " " + "MODE " + channel + " " + flag + 't');
  sendToAll(msg, itChannel);
}

void Server::operatorFlag(std::vector<std::string> CMD, Client &client,
                          char signal, size_t index) {
  std::string msg;
  std::vector<Channel>::iterator itChannel;
  std::vector<Client>::iterator itClient;
  std::vector<Client>::iterator User;
  bool mode;

  if (CMD.size() < index + 2) return; // Not enough arguments
  itChannel = searchChannel(CMD[1]);
  if (itChannel == _Channels.end()) return;  // NOT FOUND
  itClient = itChannel->searchClient(CMD[index + 1]);
  if (itClient == itChannel->endUsers()) return;  // NOT FOUND
  User = itChannel->searchClient(client.getFD());
  if (User == itChannel->endUsers()) return;  // User not in channel
  signal == '+' ? mode = true : mode = false;
  // std::cout << "OP: " << itClient->isOP() << " " << User->isOP()  << " " << mode << std::endl;
  if (!User->isOP() || itClient->isOP() == mode) return;  // Mode already set or client dont have OP
  itClient->setOP(mode);
  msg = msgMode(CMD, client,
                (mode == true) ? "+o " + CMD[index + 1] : "-o " + CMD[index + 1]);
  sendToAll(msg, itChannel);
}

void Server::userLimitFlag(std::vector<std::string> CMD, Client &client,
                           char signal, size_t index) {
  std::string msg;
  std::vector<Channel>::iterator itChannel;
  std::vector<Client>::iterator itClient;
  
  bool mode;
  int limit;

  if (CMD.size() < index + ((signal == '+') ? 2 : 1)) return; // Not enough arguments
  itChannel = searchChannel(CMD[1]);
  if (itChannel == _Channels.end()) return;  // CHANNEL NOT FOUND
  itClient = itChannel->searchClient(client.getFD());
  if (itClient == itChannel->endUsers() || !itClient->isOP()) return;  // USER NOT FOUND OR MISSING OP
  (signal == '+') ? mode = true : mode = false;
  if (!mode ){
   itChannel->setLimitMode(false);
   return;
  }
  limit = atoll(CMD[index + 1].c_str());
  if (limit <= 0 || limit > std::numeric_limits<int>::max() || (size_t)limit == itChannel->getLimit()) return;  // Overflow or Invalid Argument
  std::cout << "LIMIT: " << limit << std::endl;
  std::cout << "INPUT: " << CMD[index + 1] << std::endl;
  itChannel->setLimit(limit);
  itChannel->setLimitMode(true);
  msg = ":" + client.getNick() + '!' + client.getUser() + ' ' + CMD[0] + ' ' +
        CMD[1] + " ";
  msg += (mode ? "+l " : "-l ");
  std::ostringstream oss;
  oss << limit;
  if (mode) msg += oss.str(); 
  std::cout << "MSG: " << msg << std::endl;
  sendToAll(msg, itChannel);
}

void Server::passwordFlag(std::vector<std::string> CMD, Client &client,
                          char signal, size_t index) {
  std::string msg;
  std::vector<Channel>::iterator itChannel;
  std::vector<Client>::iterator itClient;
  bool mode;
  signal == '+' ? mode = true : mode = false;

  if (mode && CMD.size() < index + 2) return;  // Not enough arguments
  itChannel = searchChannel(CMD[1]);
  if (itChannel == _Channels.end()) return;  // Channel Not Found
  itClient = itChannel->searchClient(client.getNick());
  if (itClient == itChannel->endUsers()) return;  // User Not Found
  if (!itClient->isOP()) return;                  // Not OP

  if (mode) {
    for (int i = 0; CMD[index + 1][i]; i++) {
      if (!isalnum(CMD[index + 1][i])) return;  // Invalid Password
    }
    if (itChannel->getPassword() == CMD[index + 1]) return;  // Same Password 
    itChannel->setPassword(CMD[index + 1]);
    msg = ":" + client.getNick() + '!' + client.getUser() + ' ' + CMD[0] + ' ' +
        CMD[1] + ' ' + "+k " + CMD[index + 1];
  } else {
    if (itChannel->getPassword() == "") return;  // No password set
    itChannel->setPassword("");
    msg = ":" + client.getNick() + '!' + client.getUser() + ' ' + CMD[0] + ' ' +
        CMD[1] + ' ' + "-k";
  }
  sendToAll(msg, itChannel);
}

std::string Server::printArgs(std::vector<std::string> CMD, Client &client) {
  std::string flags, msg, ip;
  std::vector<Channel>::iterator itChannel;
  std::vector<Client>::iterator itClient;

  ip = IP;
  if (CMD.size() != 2) return "";  // INVALID ARGS
  itChannel = searchChannel(CMD[1]);
  if (itChannel == _Channels.end()) return "";  // NOT FOUND
  itClient = itChannel->searchClient(client.getNick());
  if (itClient == _Clients.end()) return "";  // NOT FOUND
  flags = '+';
  if (itChannel->getInvMode()) flags += 'i';
  if (itChannel->getTopicMode()) flags += 't';
  if (itChannel->getPassword() != "") flags += 'k';
  if (itChannel->getLimitMode()) flags += 'l';
  msg = ":IRC 324 " + client.getNick() + " " + CMD[1] + " " + flags;
  sendMessage(msg, client.getFD());
  msg = ":IRC 329 " + client.getNick() + " " + CMD[1] + " " + flags;
  sendMessage(msg, client.getFD());
  return flags;
}

std::string Server::msgMode(std::vector<std::string> CMD, Client client,
                            std::string parameter) {
  std::string msg;

  msg = ":" + client.getNick() + '!' + client.getUser() + ' ' + CMD[0] + ' ' +
        CMD[1] + ' ' + parameter;
  return msg;
}

void Server::outOfChannels(Client &clients) {
  for (std::vector<Channel>::iterator it = _Channels.begin();
       it != _Channels.end(); it++) {
    for (std::vector<Client>::iterator ic = it->beginUsers();
         ic != it->endUsers(); ic++) {
      if (ic->getFD() == clients.getFD()) it->removeUser(ic->getFD());
       break ;
    }
  }
}

static void checkPassword( std::string password ){
  if (password.empty()) {
    std::cerr << "Password is empty" << std::endl;
    exit (1);
  }
  for (size_t i = 0; i < password.size(); i++){
    if (isspace(password[i])){
      std::cerr << "Password has spaces" << std::endl;
      exit (1);
    }
  }
  return ;
}

void Server::sendToAll(std::string message, std::vector<Channel>::iterator itChannel) {
  for (std::vector<Client>::iterator it = itChannel->beginUsers();
       it != itChannel->endUsers(); it++) {
    sendMessage(message, it->getFD());
  }
}

void Server::closeServer() {
  for (std::vector<Client>::iterator it = _Clients.begin();
       it != _Clients.end(); it++) {
    close(it->getFD());
  }
  close(_serverSocket);
}