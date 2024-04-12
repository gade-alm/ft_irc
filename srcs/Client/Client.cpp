#include "Client.hpp"

Client::Client() : _nickname(""), _username(""), _operator(false) {}

Client::Client(int fd)
    : _nickname(""),
      _username(""),
      _operator(false),
      _clientfd(fd),
      _authenticated(false) {}

Client::~Client() {}

Client::Client(const Client& copy) { *this = copy; }

Client& Client::operator=(const Client& copy) {
  _nickname = copy._nickname;
  _username = copy._username;
  _operator = copy._operator;
  _authenticated = copy._authenticated;
  _registred = copy._registred;
  _clientfd = copy._clientfd;
  _client_address = copy._client_address;
  return *this;
}

const std::string Client::getNick() const { return _nickname; }
void Client::setNick(std::string Nick) { _nickname = Nick; }

const std::string Client::getUser() const { return _username; }

void Client::setUser(std::string User) { _username = User; }

int Client::getFD() const { return _clientfd; }

void Client::setFD(int FD) { _clientfd = FD; }

bool Client::isOP() const { return _operator; }

void Client::setOp(bool op) { _operator = op; }

void Client::connect(std::string password) {
  char buffer[1024];
  ssize_t n = recv(_clientfd, buffer, sizeof(buffer) - 1, 0);
  std::cout << buffer << std::endl;
  if (n == -1)
    throw(std::runtime_error("Error. Failed in rcv."));
  else if (n == 0) {
    std::cout << "Client Disconected";
    return;
  }
  buffer[n] = 0;
  std::string input = buffer;
  checkPass(password, input);
}

bool Client::getAuth() { return _authenticated; }

void Client::setAuth(bool auth) { _authenticated = auth; }

void Client::setReg(bool reg) { _registred = reg; }

bool Client::getReg() { return _registred; }

bool Client::checkPass(std::string password, std::string input) {
  std::string message;
  size_t found = input.find("PASS");
  if (found == std::string::npos) {
    message = "Server needs a password try to login with one.";
    sendMessage(message, _clientfd);
    // disconnect();
    return false;
  }
  size_t end = input.find('\n', found + 4);
  std::string afterPass = input.substr(found + 5, end - (found + 5) - 1);
  if (!afterPass.empty() && afterPass[0] == ':') afterPass.erase(0, 1);
  if (afterPass != password) {
    message = "Wrong password.";
    sendMessage(message, _clientfd);
    // disconnect();
    return false;
  }
  message = "You are authenticated. Welcome.";
  sendMessage(message, _clientfd);
  return true;
}

bool Client::checkNick(std::string input, std::vector<Client>& Clients) {
  std::string message;
  size_t found = input.find("NICK");
  // std::cout << "AQUI" << std::endl;
  if (found == std::string::npos) {
    message = "You must have a nickname to join this server.";
    sendMessage(message, _clientfd);
    return false;
  }
  size_t end = input.find('\n', found + 4);
  std::string afterNick = input.substr(found + 5, end - (found + 5));
  if (!afterNick.empty() && afterNick[0] == ':') afterNick.erase(0, 1);
  std::vector<Client>::iterator it;
  for (it = Clients.begin(); it != Clients.end(); ++it) {
    if (it->getNick() == afterNick && it->getFD() != _clientfd) {
      message = "That nickname was already chosen.";
      sendMessage(message, _clientfd);
      return false;
    }
  }
  _nickname = afterNick;
  return true;
}

bool Client::checkName(std::string input) {
  std::string message;
  size_t found = input.find("USER");
  if (found == std::string::npos) {
    message = "You must have a username to join this server.";
    sendMessage(message, _clientfd);
    return false;
  }
  size_t end = input.find(' ', found + 5);
  std::string afterUser = input.substr(found + 5, end - (found + 5));
  if (!afterUser.empty() && afterUser[0] == ':') afterUser.erase(0, 1);
  _username = afterUser;
  return true;
}

void Client::authenticateClient(std::string password, std::string input,
                                std::vector<Client>& Clients) {
  if (!checkPass(password, input)) return;
  if (!checkNick(input, Clients)) return;
  if (!checkName(input)) return;
  sendMessage("Welcome.", _clientfd);
  _authenticated = true;
  std::cout << "_nickname: " << _nickname << std::endl;
  std::cout << "_username: " << _username << std::endl;
  // std::cout << "NICK: " << _nickname <<  "USER: "
}

void Client::disconnect() {
  std::string message = "You have been disconnected.";
  sendMessage(message, _clientfd);
  if (close(_clientfd) == -1) perror("close");
}

void sendMessage(std::string string, int fd) {
  string += "\r\n";
  if (send(fd, string.c_str(), string.size(), 0) == -1) perror("send");
}
