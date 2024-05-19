#ifndef SERVER_HPP
#define SERVER_HPP

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

#include "Channel.hpp"
#include "Client.hpp"

#define MINPORT 1024
#define MAXPORT 65535
#define BACKLOG 10
#define IP "127.0.0.1"
#define PROTOCOL 0
#define MAXUSERS 24

class Server {
 private:
  Server(const Server &);
  Server();

  int _serverSocket;
  int _listenSocket;
  int _bindSocket;

  int _serverPort;
  int _clientfd;
  std::string _password;

  fd_set _selectfds;
  fd_set _masterfds;
  std::vector<Client> _Clients;
  std::vector<Channel> _Channels;
  std::vector<std::string> parseCMD(std::string buffer);

 public:
  int maxfds;
  char buf[1024];
  sockaddr_in serverAddr;
  Server(const char *portValue, const std::string &passwordValue);
  void inviteOnly(std::vector<std::string> CMD, Client &client, bool plus);
  void topicFlag(std::vector<std::string> CMD, Client &client, bool plus);
  void operatorFlag(std::vector<std::string> CMD, Client &client, bool plus);
  void userLimitFlag(std::vector<std::string> CMD, Client &client, bool plus);
  void passwordFlag(std::vector<std::string> CMD, Client &client, bool plus);
  std::string printArgs(std::vector<std::string> CMD, Client &client);
  std::string msgMode(std::vector<std::string> CMD, Client client, std::string parameter);

  void setSocket(int socketFd);
  void setBind(void);
  void initAddr(void);
  void listenSockets(void);
  void prepareFDs(void);
  void selectLoop(int i, struct sockaddr_in _clientaddr, int numbytes);
  int getSocket(void);
  void cmdHandler(std::string buffer, Client &client);
  void joinChannel(std::vector<std::string> CMD, Client &client);
  void quitServer(std::vector<std::string> CMD, Client &client);
  void deliveryMSG(std::vector<std::string> CMD, Client &client);
  void kickFromChannel(std::vector<std::string> CMD, Client &client);
  void topicChannel(std::vector<std::string> CMD, Client &client);
  bool channelPrep(std::string channelname, Client &client);
  std::vector<Client>::iterator searchClient(int fd);
  std::vector<Client>::iterator searchClient(std::string name);
  std::vector<Channel>::iterator searchChannel(std::string channelname);
  void disconnectClient(std::vector<Client>::iterator it);
  void invite(std::vector<std::string> CMD, Client &client);
  void mode(std::vector<std::string> CMD, Client &client);

  ~Server();
};

#endif
