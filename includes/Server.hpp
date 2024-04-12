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

#include "Client.hpp"

#define MINPORT 1024
#define MAXPORT 65535
#define BACKLOG 10
#define IP "127.0.0.1"
#define BOSS "SOU MAQUINA"
#define PROTOCOL 0
#define MAXUSERS 24

class Server {
 private:
  Server(const Server&);
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

 public:
  int maxfds;
  char buf[1024];
  sockaddr_in serverAddr;
  Server(const char* portValue, const std::string& passwordValue);

  void setSocket(int socketFd);
  void setBind(void);
  void initAddr(void);
  void listenSockets(void);
  void prepareFDs(void);
  void selectLoop(int i, struct sockaddr_in _clientaddr, int numbytes);
  int getSocket(void);
  std::vector<Client>::iterator searchClient(int fd);
  void disconnectClient(std::vector<Client>::iterator it);

  ~Server();
};

#endif
