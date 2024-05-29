#include "Client.hpp"

Client::Client() : _nickname(""), _username(""), _operator(false) {}

Client::Client(int fd)
    : _nickname(""),
      _username(""),
      _operator(false),
      _clientfd(fd),
      _authenticated(false),
      _registred (false) {}

Client::~Client() {}

Client::Client(const Client& copy) { *this = copy; }

Client& Client::operator=(const Client& copy) {
  _nickname = copy._nickname;
  _username = copy._username;
  _operator = copy._operator;
  _authenticated = copy._authenticated;
  _registred = copy._registred;
  _clientfd = copy._clientfd;
  return *this;
}

bool Client::operator==(const Client& copy) const {
  return _clientfd == copy._clientfd;
}
const std::string Client::getNick() const { return _nickname; }
void Client::setNick(std::string Nick) { _nickname = Nick; }

const std::string Client::getUser() const { return _username; }

void Client::setUser(std::string User) { _username = User; }

int Client::getFD() const { return _clientfd; }

void Client::setFD(int FD) { _clientfd = FD; }

bool Client::isOP() const { return _operator; }

void Client::setOP(bool op) { _operator = op; }

void Client::connect(std::string password) {
  char buffer[1024];
  ssize_t n = recv(_clientfd, buffer, sizeof(buffer) - 1, 0);
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

bool Client::checkPass(std::string password, std::string input){
    std::string message;
    size_t found = input.find("PASS");
	size_t end;
	std::string afterPass;

    if (found == std::string::npos) return false;
    end = input.find('\r', found + 5);
	if (end == std::string::npos)
		end = input.find('\n', found + 5);
    afterPass = input.substr(found + 5, end - (found + 5));
    if (afterPass.empty()) {
      message = ":IRC 464 Nick :Password is empty\r\n";
      sendMessage(message, _clientfd);
      return false;
    }
    for (size_t i = 0; i < afterPass.size(); i++) {
      if (isspace(afterPass[i])) {
      message = ":IRC 464 Nick :Password with spaces\r\n";
      sendMessage(message, _clientfd);
      return false;
      }
    }
    if (!afterPass.empty() && afterPass[0] == ':')
        afterPass.erase(0, 1);
    if (afterPass != password){
        message = ":IRC 464 Nick :Password incorrect\r\n";
        sendMessage(message, _clientfd);
        return false;
    }
    return true;
}

bool Client::checkNick(std::string input, std::vector<Client> &Clients){
    std::string message;
    size_t found = input.find("NICK");
	size_t end;
	std::string afterNick;

    if (found == std::string::npos) return false;

    end = input.find('\r', found + 5);
	if (end == std::string::npos)
		end = input.find('\n', found + 5);
    afterNick = input.substr(found + 5, end - (found + 5));
    if (afterNick.empty()) {
      message = ":IRC 464 Nick :Nick is empty\r\n";
      sendMessage(message, _clientfd);
      return false;
    }
    for (size_t i = 0; i < afterNick.size(); i++) {
      if (isspace(afterNick[i])) {
      message = ":IRC 464 Nick :Nick with spaces\r\n";
      sendMessage(message, _clientfd);
      return false;
      }
    }
    if (!afterNick.empty() && afterNick[0] == ':')
        afterNick.erase(0, 1);
    std::vector<Client>::iterator it;
	for (it = Clients.begin(); it != Clients.end(); ++it) {
    	if (it->getNick() == afterNick && it->getFD() != _clientfd){
            message = "That nickname was already chosen.";
            sendMessage(message, _clientfd);
       		return false;
        }
	}
    _nickname = afterNick;
    return true;
}

bool Client::checkName(std::string input){
    std::string message;
    size_t found = input.find("USER");
	size_t end;
	std::string afterUser;

	if (found == std::string::npos) return false;
    end = input.find(' ', found + 5);
		if (end == std::string::npos)
		end = input.find('\n', found + 5);
    afterUser = input.substr(found + 5, end - (found + 5));
    if (afterUser.empty()) {
      message = ":IRC 464 Nick :User is empty\r\n";
      sendMessage(message, _clientfd);
      return false;
    }
    if (!afterUser.empty() && afterUser[0] == ':')
        afterUser.erase(0, 1);
    _username = afterUser;
    return true;
}

bool Client::authenticateClient(std::string password, std::string input, std::vector<Client> &Clients){
    if (_authenticated == true)
        return true;
    if (checkPass(password, input))
        _registred = true;
    checkNick(input, Clients);
    checkName(input);
	std::cout << "Input: " << input << std::endl;
    std::cout << "Nick: " << _nickname << std::endl;
    std::cout << "Name: " << _username << std::endl;
    if (!_nickname.empty() && !_username.empty() && _registred){
        sendMessage("You are authenticated. Welcome.", _clientfd);
        _authenticated = true;
        return true;
    }
    return false;
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
