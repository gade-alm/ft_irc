#include "Server.hpp"

#include <algorithm>

// static void parserTest ( std::string buffer, int i );
Server::Server(void) {}

Server::Server(const Server &) {}

Server::~Server(void) {}

Server::Server(const char *portValue, const std::string &passwordValue) {
  _clientfd = 0;
  maxfds = 0;
  int used = 1;
  int port = atoi(portValue);
  if (port < MINPORT || port > MAXPORT) {
    std::cout << ("Wrong number on port") << std::endl;
    return;
  }
  _serverPort = port;
  _password = passwordValue;
  initAddr();
  setSocket(socket(serverAddr.sin_family, SOCK_STREAM, PROTOCOL));
  fcntl(_serverSocket, F_SETFL, O_NONBLOCK);

  if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &used,
                 sizeof(used)) == -1) {
    perror("setsockopt");
    return;
  }

  setBind();

  listenSockets();
}

void Server::setSocket(int socketFD) {
  _serverSocket = socketFD;
  if (_serverSocket == -1) {
    perror("setSocket");
    return;
  }
}

void Server::setBind(void) {
  _bindSocket = bind(_serverSocket, (struct sockaddr *)&serverAddr,
                     sizeof(struct sockaddr));
  if (_bindSocket == -1) {
    perror("setBind");
    exit(1);
  }
}

void Server::initAddr(void) {
  memset(&(serverAddr.sin_zero), 0, sizeof(serverAddr.sin_zero));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(_serverPort);
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  if (serverAddr.sin_addr.s_addr != 0) {
    perror("initAddr");
    return;
  }
}

void Server::listenSockets(void) {
  _listenSocket = listen(_serverSocket, BACKLOG);
  if (_listenSocket == -1) {
    perror("Error while opening sockets");
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

void Server::selectLoop(int i, struct sockaddr_in _clientaddr, int numbytes) {
  _selectfds = _masterfds;
  if (select(maxfds + 1, &_selectfds, NULL, NULL, NULL) == -1) {
    // perror ("select error");
    return;
  }
  if (FD_ISSET(i, &_selectfds) != 0) {
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
      if ((numbytes = recv(i, buf, sizeof(buf), 0)) < 1) {
        perror("recv error");
        close(i);
        FD_CLR(i, &_masterfds);
      } else {
        std::string buffer(buf, numbytes);
        // std::cout << buffer << std::endl;
        std::vector<Client>::iterator it = searchClient(i);
        if (it != _Clients.end()) {
          Client &client = *it;
          if (!client.authenticateClient(_password, buf, _Clients)) {
            disconnectClient(it);
            return;
          }
          // std::cout << "CLIENTFD: " << client.getFD() << " is Auth: " <<
          // client.getAuth() << std::endl;
          cmdHandler(buffer, client);
        } else {
          disconnectClient(it);
          return;
        }
      }
    }
  }
  memset(buf, 0, sizeof(buf));
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
  for (it = _Channels.begin(); it != _Channels.end(); ++it) {
    // std::cout << "ClientName: " << it->getName() << " SIZE: "<<
    // it->getName().size() << std::endl;
    // std::cout << "Name: " << channelname << " SIZE: "<< channelname.size() <<
    // std::endl;
    if (it->getName() == channelname) {
      // std::cout << "TRUE CHANNEL" << std::endl;
      break;
    }
  }
  return it;
}

void Server::disconnectClient(std::vector<Client>::iterator it) {
  Client &client = *it;
  FD_CLR(client.getFD(), &_masterfds);
  std::string message = "You have been disconnected.";
  sendMessage(message, client.getFD());
  close(client.getFD());
  _Clients.erase(it);
}

void Server::cmdHandler(std::string buffer, Client &client) {
  std::cout << buffer << std::endl;
  std::vector<std::string> CMD = parseCMD(buffer);
  std::string cmd = buffer.substr(0, buffer.find(" "));
  void (Server::*myCMDS[7])(std::vector<std::string>, Client &) = {
      &Server::joinChannel,  &Server::quitServer,
      &Server::deliveryMSG,  &Server::kickFromChannel,
      &Server::topicChannel, &Server::invite,
      &Server::mode};
  long unsigned int index;
  std::string cmds[7] = {"JOIN",  "QUIT",   "PRIVMSG", "KICK",
                         "TOPIC", "INVITE", "MODE"};

  for (index = 0; index < sizeof(cmds) / sizeof(cmds[0]); index++) {
    if (cmd == cmds[index]) break;
  }
  if (index < sizeof(cmds) / sizeof(cmds[0]))
    (this->*myCMDS[index])(CMD, client);
}

void Server::joinChannel(std::vector<std::string> CMD, Client &client) {
  std::string channelname = CMD[1];
  if (channelname[0] != '#') channelname = "#" + channelname;
  if (!channelPrep(channelname, client)) {
    // Mensagem de erro
    return;
  };
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

bool Server::channelPrep(std::string channelname, Client &client) {
  std::vector<Channel>::iterator itChannel = searchChannel(channelname);

  if (itChannel != _Channels.end()) {
    // Caso esteja em Invite Mode e sem invitation retornar logo.
    if (itChannel->getInvMode() &&
        std::find(itChannel->_invitation.begin(), itChannel->_invitation.end(),
                  client.getFD()) == itChannel->_invitation.end()) {
      return false;
    }
    if (itChannel->searchClient(client.getFD()) == itChannel->endUsers())
      itChannel->addUser(client);
    return true;
  }
  Channel channel(channelname);
  channel.addUser(client);
  std::vector<Client>::iterator itClient = channel.searchClient(client.getFD());
  itClient->setOp(true);
  _Channels.push_back(channel);

  itChannel = searchChannel(channelname);
  // itChannel->printUsers();
  return true;
}

std::string prepReason(std::vector<std::string> CMD, int i) {
  std::string reason;
  for (std::vector<std::string>::iterator it = CMD.begin() + i; it != CMD.end();
       it++) {
    if (it + 1 != CMD.end()) reason += ' ';
    reason += *it;
  }
  return reason;
}

void Server::deliveryMSG(std::vector<std::string> CMD, Client &client) {
  // PRIVMSG Gabriel :oh
  std::string channelname = CMD[1];
  std::string msg = prepReason(CMD, 2);
  // std::cout << "CHANNEL NAME: " << channelname << std::endl;
  std::string message = ":" + client.getNick() + "!~" + client.getUser() +
                        " PRIVMSG " + channelname + " " + msg;
  // std::cout << "MESSAGE: " << message << std::endl;
  // std::cout << itChannel->getName() << std::endl;
  if (channelname[0] != '#') {
    std::vector<Client>::iterator itClient = searchClient(channelname);
    sendMessage(message, itClient->getFD());
    return;
  }

  std::vector<Channel>::iterator itChannel = searchChannel(channelname);
  for (std::vector<Client>::iterator itClient = itChannel->beginUsers();
       itClient != itChannel->endUsers(); itClient++) {
    if (itClient->getFD() != client.getFD())
      sendMessage(message, itClient->getFD());
  }
}

void Server::kickFromChannel(std::vector<std::string> CMD, Client &client) {
  // KICK #channelname username reason
  // :server_name KICK #channelname username :reason
  std::string channelname = CMD[1];
  std::string nick = CMD[2];
  std::string reason = (CMD.size() >= 4) ? prepReason(CMD, 3) : "";
  std::vector<Channel>::iterator it = searchChannel(channelname);
  if (it == _Channels.end()) return;
  if (!it->searchClient(client.getNick())->isOP()) {
    sendMessage("You dont have the rights to kick Users.", client.getFD());
    return;
  }
  if (it->searchClient(nick) == it->endUsers()) {
    sendMessage("User not found.", client.getFD());
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
  size_t end;
  std::string word;

  std::vector<std::string> CMD;
  // std::cout << "ENTROU" << std::endl;
  while (end != buffer.size() - 2) {
    end = buffer.find(" ", start);
    if (end == std::string::npos) {
      end = buffer.find("\r", start);
      word = buffer.substr(start, end - start);
      CMD.push_back(word);
      // std::cout << "WORD: " << word << " SIZE " << word.size() << std::endl;
      break;
    }
    word = buffer.substr(start, end - start);
    CMD.push_back(word);
    // std::cout << "WORD: " << word << " SIZE " << word.size() << std::endl;
    start = end + 1;
  }
  return CMD;
}

void Server::topicChannel(std::vector<std::string> CMD, Client &client) {
  std::string channelName = CMD[1];
  std::vector<Channel>::iterator it = searchChannel(channelName);
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

  it->setTopic(topic.substr(1, topic.size() - 1));
  for (std::vector<Client>::iterator itClient = it->beginUsers();
       itClient != it->endUsers(); itClient++) {
    sendMessage(msg, itClient->getFD());
  }
}

// Invite Function
void Server::invite(std::vector<std::string> CMD, Client &client) {
  std::vector<Client>::iterator destiny = searchClient(CMD[1]);
  std::vector<Channel>::iterator channel = searchChannel(CMD[2]);
  std::vector<Client>::iterator original;
  int fd;
  std::string msg;

  if (channel == _Channels.end()) {
    // Mensagem de erro
    return;
  }
  original = std::find(channel->beginUsers(), channel->endUsers(), client);
  if (destiny == _Clients.end() && original->isOP()) {
    std::string msg = ":IRC 482 " + client.getNick() + " " +
                      channel->getName() + " :You're not channel operator\r\n";
    return;
  }
  fd = destiny->getFD();
  channel->_invitation.push_back(client.getFD());
  msg = ':' + client.getNick() + '!' + client.getUser() + "@127.0.0.1 " +
        CMD[0] + " " + CMD[1] + " " + CMD[2] + "\r\n";
  sendMessage(msg, destiny->getFD());
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
  std::string cmds = "itkol";
  void (Server::*myCMDS[5])(std::vector<std::string>, Client &, bool) = {
      &Server::inviteOnly, &Server::topicFlag, &Server::passwordFlag,
      &Server::operatorFlag, &Server::userLimitFlag};
  std::string flags;

  // Com dois argumentos printar as permissoes
  if (CMD.size() == 2) {
    flags = printArgs(CMD, client);
  }
  // Com mais setar permissoes
  // Caso haja um k vai haver mais um argumento que vai ser a password
  for (int i = 2; i < CMD.size() && (CMD[i][0] != '+' || CMD[i][0] != '-');
       i++) {
    for (int j = 1; j < CMD[i].size(); i++) {
      flags += CMD[i][0];
      if (CMD[i][0] == '+') {
        if (flags.find(CMD[i][j]) == std::string::npos) flags += CMD[i][j];
      } else {
        try {
          size_t index = CMD[i][j];

          flags.erase(flags.find(index));
          flags.erase(flags.find(index));
        } catch (std::exception &e) {
        }
      }
    }
  }
  for (int i = 0; i < flags.size(); i += 2) {
    for (int j = 0;
         i < flags.size() - 1 && cmds.find(flags[i + 1]) == std::string::npos;
         i++) {
    }
    (this->*myCMDS[i])(CMD, client, (flags[i] == '+'));
  }
}

void Server::inviteOnly(std::vector<std::string> CMD, Client &client,
                        bool plus) {
  std::string channel = CMD[1];
  std::vector<Channel>::iterator itChannel = searchChannel(channel);
  bool mode;

  if (itChannel == _Channels.end()) return;
  plus ? mode = true : mode = false;
  if (itChannel->getInvMode() == mode) return;
  itChannel->setInvMode(mode);
}

void Server::topicFlag(std::vector<std::string> CMD, Client &client,
                       bool plus) {
  std::string channel = CMD[1];
  std::vector<Channel>::iterator itChannel = searchChannel(channel);
  bool mode;

  if (itChannel == _Channels.end()) return;
  plus ? mode = true : mode = false;
  if (itChannel->getTopicMode() == mode) return;
  itChannel->setTopicMode(mode);
}

void Server::operatorFlag(std::vector<std::string> CMD, Client &client,
                          bool plus) {
  std::string channel = CMD[1];
  std::vector<Channel>::iterator itChannel = searchChannel(channel);
  std::vector<Client>::iterator itClient = searchClient(client.getFD());
  bool mode;

  if (itChannel == _Channels.end()) return;
  plus ? mode = true : mode = false;
  if (itClient->isOP() == mode) return;
  itClient->setOp(mode);
}

void Server::userLimitFlag(std::vector<std::string> CMD, Client &client,
                           bool plus) {
  std::string channel = CMD[1];
  std::vector<Channel>::iterator itChannel = searchChannel(channel);
  bool mode;
  std::string parameter = (plus == true) ? "+l" : "-l";

  if (itChannel == _Channels.end()) return;
  plus ? mode = true : mode = false;
  if (itChannel->getLimitMode() == mode) return;
  itChannel->setLimitMode(mode);
}

void Server::passwordFlag(std::vector<std::string> CMD, Client &client,
                          bool plus) {
  std::string channel = CMD[1];
  std::vector<Channel>::iterator itChannel = searchChannel(channel);
  int i;

  (void)plus;
  for (i = 2; i < CMD.size() && (CMD[i][0] != '+' || CMD[i][0] != '-'); i++);
  if (itChannel->getPassword() == "") itChannel->setPassword(CMD[i]);
  sendMessage(msg, client.getFD());
}

std::string Server::printArgs(std::vector<std::string> CMD, Client &client) {
  std::string flags;
  std::vector<Channel>::iterator itChannel = searchChannel(CMD[1]);

  flags = '+';
  if (itChannel->getInvMode()) flags += 'i';
  if (itChannel->getTopicMode()) flags += 't';
  if (itChannel->getPassword() != "") flags += 'k';

  std::string msg =
      ':' + "127.0.0.1 324" + client.getNick() + ' ' + CMD[1] + flags;
  sendMessage(msg, client.getFD());
  return flags;
}

std::string Server::msgMode(std::vector<std::string> CMD, Client client,
                            std::string parameter) {
  std::string msg;

  msg = ":" + client.getNick() + '!' + client.getUser() + "127.0.0.1" + ' ' +
        CMD[0] + ' ' + CMD[1] + ' ' + parameter;
  return msg;
}
