#include "Server.hpp"

#include <algorithm>
#include <sstream>
#include <string>

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

void Server::selectLoop(struct sockaddr_in _clientaddr) {
  _selectfds = _masterfds;
  if (select(maxfds + 1, &_selectfds, NULL, NULL, NULL) == -1) {
    perror("SELECT:");
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
            perror("recv error");
            if (searchClient(i) != _Clients.end()) {
              disconnectClient(searchClient(i));
              return;
            }
            close(i);
            FD_CLR(i, &_masterfds);
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
  for (it = _Clients.begin(); it != _Clients.end(); it++) {
    if (it->getFD() == fd) break;
  }
  return it;
}

std::vector<Client>::iterator Server::searchClient(std::string name) {
  std::vector<Client>::iterator it;
  for (it = _Clients.begin(); it != _Clients.end(); it++) {
    if (it->getNick() == name) break;
  }
  return it;
}

std::vector<Channel>::iterator Server::searchChannel(std::string channelname) {
  std::vector<Channel>::iterator it;
  for (it = _Channels.begin(); it != _Channels.end(); it++) {
    if (it->getName() == channelname) {
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
  // std::cout << buffer << std::endl;
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

bool Server::channelPrep(std::string channelname, Client &client) {
  std::vector<Channel>::iterator itChannel = searchChannel(channelname);

  if (itChannel != _Channels.end()) {
    // Caso esteja em Invite Mode e sem invitation retornar logo.
    itChannel->setInvMode(true);
    std::cout << "CLIENT FD: " << client.getFD() << " "
              << itChannel->_invitation[0] << std::endl;
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
  for (std::vector<Client>::iterator itClient = itChannel->beginUsers();
       itClient != itChannel->endUsers(); itClient++) {
    if (itClient->getFD() != client.getFD())
      sendMessage(message, itClient->getFD());
  }
}

void Server::kickFromChannel(std::vector<std::string> CMD, Client &client) {
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
  size_t end = 0;
  std::string word;
  std::vector<std::string> CMD;

  while (end != buffer.size() - 2) {
    end = buffer.find(" ", start);
    if (end == std::string::npos) {
      end = buffer.find("\r", start);
      word = buffer.substr(start, end - start);
      CMD.push_back(word);
      break;
    }
    word = buffer.substr(start, end - start);
    CMD.push_back(word);
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
  channel->_invitation.push_back(client.getFD());
  msg = ':' + client.getNick() + '!' + client.getUser() + "@127.0.0.1 " +
        CMD[0] + " " + CMD[1] + " " + CMD[2] + "\r\n";
  sendMessage(msg, destiny->getFD());
}

/**********************
 * Auxiliar Functions *
 **********************/
std::vector<std::pair<std::string, size_t> >::iterator searchFlagDiffSign(
    std::vector<std::pair<std::string, size_t> >::iterator begin,
    std::vector<std::pair<std::string, size_t> >::iterator end, char option,
    char flag) {
  for (; begin != end; begin++) {
    if (begin->first[0] != option && begin->first[1] == flag) return begin;
  }
  return begin;
}

std::vector<std::pair<std::string, size_t> >::iterator searchFlagEqSign(
    std::vector<std::pair<std::string, size_t> >::iterator begin,
    std::vector<std::pair<std::string, size_t> >::iterator end, char option,
    char flag) {
  for (; begin != end; begin++) {
    if (begin->first[0] == option && begin->first[1] == flag) return begin;
  }
  return begin;
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
  void (Server::*myCMDS[5])(std::vector<std::string>, Client &, bool,
                            size_t) = {
      &Server::inviteOnly, &Server::topicFlag, &Server::passwordFlag,
      &Server::operatorFlag, &Server::userLimitFlag};
  std::vector<std::pair<std::string, size_t> >::iterator itflag;
  std::vector<std::pair<std::string, size_t> > flags;
  std::pair<std::string, size_t> flag;
  size_t numArgs = 0, indexArgs = 2, indexSign = 0;

  // Com dois argumentos printar as permissoes
  if (CMD.size() == 3) {
    printArgs(CMD, client);
    return;
  }
  // Algoritmo Para tratar flags
  for (size_t i = 0; i < CMD.size(); i++) {
    if (CMD[i][0] != '-' && CMD[i][0] != '+') continue;
    for (size_t j = 0; j < CMD[0].size(); i++) {
      if (CMD[i][j] == '-' || CMD[i][j] == '+') {
        indexSign = j;
        continue;
      }
      flag.first = "";
      flag.second = 0;
      // Procurar por se existe outra flag no vector
      if (CMD[i][j] == 'k' || CMD[i][j] == 'o' ||
          (CMD[i][j] == 'l' && CMD[i][indexSign] == '+')) {
        // Procurar por arg se nao encontrar mandar erro
        for (size_t k = indexArgs; k < CMD.size(); k++) {
          if (CMD[k][0] != '-' && CMD[k][0] != '+') {
            numArgs++;
            indexArgs = k;
            break;
          }
          if (k == CMD.size() - 1) indexArgs = 0;
        }
        if (indexArgs == 0) {
          // Mandar mensagem: Erro Nao tem argumentos
          return;
        }
        flag.second = indexArgs;
      }
      flag.first = CMD[i][indexSign] + CMD[i][j];
      itflag = searchFlagDiffSign(flags.begin(), flags.end(), flag.first[0],
                                  flag.first[1]);
      if (itflag != flags.end()) flags.erase(itflag);
      itflag = searchFlagEqSign(flags.begin(), flags.end(), flag.first[0],
                                flag.first[1]);
      if (itflag == flags.end()) flags.insert(flags.begin(), flag);
    }
    indexSign = 0;
  }
  // Using Flags to execute the functions
  for (itflag = flags.begin(); itflag != flags.end(); itflag++) {
    for (size_t i = 0; i < 5; i++) {
      if (cmds[i] == itflag->first[1]) {
        (this->*myCMDS[i])(CMD, client, (itflag->first[0] == '+'),
                           itflag->second);
        break;
      }
    }
  }
}

//   // Com dois argumentos printar as permissoes
//   if (CMD.size() == 2) {
//     flags = printArgs(CMD, client);
//   }
//   // Com mais setar permissoes
//   // Caso haja um k vai haver mais um argumento que vai ser a password
//   for (size_t i = 2; i < CMD.size() && (CMD[i][0] != '+' || CMD[i][0] !=
//   '-');
//        i++) {
//     for (size_t j = 1; j < CMD[i].size(); i++) {
//       if (CMD[i][0] != '+' && CMD[i][0] != '-') continue;
//       if (flags.find(CMD[i][j]) == std::string::npos) {
//         flags += CMD[i][0];
//         flags += CMD[i][j];
//         (this->*myCMDS[i])(CMD, client, (CMD[i][0] == '+'), i);
//       } else {
//         try {
//           size_t index = CMD[i][j];

//           flags.erase(flags.find(index));
//           flags.erase(flags.find(index));
//         } catch (std::exception &e) {}
//       }
//     }
//   }
//   for (size_t i = 0; i < flags.size(); i += 2) {
//     for (size_t j = 0; j < 5; j++) {

//     }
//     (this->*myCMDS[i])(CMD, client, (flags[i] == '+'), i);
//   }
// }

void Server::inviteOnly(std::vector<std::string> CMD, Client &client, bool plus,
                        size_t argsUsed) {
  std::string channel = CMD[1], msg;
  std::vector<Channel>::iterator itChannel = searchChannel(channel);
  bool mode;

  (void)argsUsed;
  if (itChannel == _Channels.end()) return;
  plus ? mode = true : mode = false;
  if (itChannel->getInvMode() == mode) return;
  itChannel->setInvMode(mode);
  msg = msgMode(CMD, client, (plus == true) ? "+i" : "-i");
  sendMessage(msg, client.getFD());
}

void Server::topicFlag(std::vector<std::string> CMD, Client &client, bool plus,
                       size_t argsUsed) {
  std::string channel = CMD[1], msg;
  std::vector<Channel>::iterator itChannel = searchChannel(channel);
  bool mode;

  (void)argsUsed;
  if (itChannel == _Channels.end()) return;
  plus ? mode = true : mode = false;
  if (itChannel->getTopicMode() == mode) return;
  itChannel->setTopicMode(mode);
  msg = msgMode(CMD, client, (plus == true) ? "+t" : "-t");
  sendMessage(msg, client.getFD());
}

void Server::operatorFlag(std::vector<std::string> CMD, Client &client,
                          bool plus, size_t argsUsed) {
  std::string channel = CMD[1], msg;
  std::vector<Channel>::iterator itChannel = searchChannel(channel);
  std::vector<Client>::iterator itClient;
  bool mode;

  if (itChannel == _Channels.end()) return;  // NOT FOUND
  itClient = itChannel->searchClient(client.getNick());
  if (itClient == _Clients.end()) return;  // NOT FOUND
  plus ? mode = true : mode = false;
  if (itClient->isOP() == mode) return;
  itClient->setOP(mode);
  msg = msgMode(CMD, client,
                (plus == true) ? "+o " + CMD[argsUsed] : "-o " + CMD[argsUsed]);
  sendMessage(msg, client.getFD());
}

void Server::userLimitFlag(std::vector<std::string> CMD, Client &client,
                           bool plus, size_t argsUsed) {
  std::string channel = CMD[1], msg;
  std::stringstream parameter;
  std::vector<Channel>::iterator itChannel = searchChannel(channel);
  bool mode;

  if (itChannel == _Channels.end()) return;  // NOT FOUND
  plus ? mode = true : mode = false;
  if (itChannel->getLimitMode() == mode) return;  // NOT FOUND
  itChannel->setLimitMode(mode);
  itChannel->setLimit(atoi(CMD[argsUsed].c_str()));
  parameter << ((plus == true) ? "+l " : "-l ");
  parameter << itChannel->getLimit();
  msg = msgMode(CMD, client, parameter.str());
  sendMessage(msg, client.getFD());
}

void Server::passwordFlag(std::vector<std::string> CMD, Client &client,
                          bool plus, size_t argsUsed) {
  std::string channel = CMD[1], msg;
  std::vector<Channel>::iterator itChannel = searchChannel(channel);
  size_t i;

  (void)plus;
  for (i = argsUsed; i < CMD.size(); i++) {
    if (CMD[i][0] != '+' && CMD[i][0] != '-') break;
  }
  if (itChannel->getPassword() == "") itChannel->setPassword(CMD[argsUsed]);
  msg = msgMode(CMD, client,
                (plus == true) ? "+k " + CMD[argsUsed] : "-k" + CMD[argsUsed]);
  sendMessage(msg, client.getFD());
}

//Mensagens com para o MODE #A
//>> :Aurora.AfterNET.Org 324 test1 #a +
//>> :Aurora.AfterNET.Org 329 test1 #a 1716325933
std::string Server::printArgs(std::vector<std::string> CMD, Client &client) {
  std::string flags, msg;
  std::vector<Channel>::iterator itChannel = searchChannel(CMD[1]);
  std::vector<Client>::iterator itClient =
      itChannel->searchClient(client.getNick());

  if (itChannel == _Channels.end()) return "";  // NOT FOUND
  if (itClient == _Clients.end()) return "";    // NOT FOUND
  flags = '+';
  if (itChannel->getInvMode()) flags += 'i';
  if (itChannel->getTopicMode()) flags += 't';
  if (itChannel->getPassword() != "") flags += 'k';
  if (itChannel->getLimitMode()) flags += 'l';
  msg =
      ":" + client.getNick() + '!' + client.getUser() + " " + CMD[0] + ' ' + CMD[1] + ' ' + flags;
  sendMessage(msg, client.getFD());
  return flags;
}

std::string Server::msgMode(std::vector<std::string> CMD, Client client,
                            std::string parameter) {
  std::string msg;

  msg = ":" + client.getNick() + '!' + client.getUser() + "@127.0.0.1" + ' ' +
        CMD[0] + ' ' + CMD[1] + ' ' + parameter;
  return msg;
}
