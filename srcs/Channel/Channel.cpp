#include "Channel.hpp"

Channel::Channel()
    : _name(""),
      _password(""),
      _topic(""),
      _topicneedop(true),
      _limit(false),
      _userlimit(0) {}

Channel::Channel(std::string name)
    : _name(name),
      _password(""),
      _topic(""),
      _topicneedop(true),
      _limit(false),
      _userlimit(0) {}

Channel::~Channel() {}

Channel::Channel(const Channel& copy) { *this = copy; }

Channel& Channel::operator=(const Channel& copy) {
  _name = copy._name;
  _password = copy._password;
  _topic = copy._topic;
  _topicneedop = copy._topicneedop;
  _inviteonly = copy._inviteonly;
  _limit = copy._limit;
  _userlimit = copy._userlimit;
  _Users = copy._Users;
  _invitation = copy._invitation;
  return *this;
}

void Channel::setName(std::string name) { _name = name; }

const std::string Channel::getName() const { return _name; }

void Channel::setPassword(std::string password) { _password = password; }

const std::string Channel::getPassword() const { return _password; }

void Channel::setTopic(std::string topic) { _topic = topic; }

const std::string Channel::getTopic() const { return _topic; }

void Channel::setInvMode(bool mode) { _inviteonly = mode; }

bool Channel::getInvMode() const { return _inviteonly; }

void Channel::setLimit(long limit) { _userlimit = limit; }

long Channel::getLimit() const { return _userlimit; }

void Channel::setLimitMode(bool mode) { _limit = mode; }

bool Channel::getLimitMode() const { return _limit; }

const std::vector<Client>& Channel::getUserOn() const { return _Users; }

void Channel::addUser(Client& client) { _Users.push_back(client); }

void Channel::rmUser(Client& client) {
  std::vector<Client>::iterator it = find(_Users.begin(), _Users.end(), client);
  if (it == _Users.end()) return;
  _Users.erase(it);
}

void Channel::setTopicMode(bool topic) { _topicneedop = topic; }

bool Channel::getTopicMode() const { return _topicneedop; }

std::vector<Client>::iterator Channel::searchClient(int fd) {
  std::vector<Client>::iterator it;
  for (it = _Users.begin(); it != _Users.end(); ++it) {
    if (it->getFD() == fd) break;
  }
  return it;
}

std::vector<Client>::iterator Channel::searchClient(std::string name) {
  std::vector<Client>::iterator it;
  for (it = _Users.begin(); it != _Users.end(); ++it) {
    if (it->getNick() == name) break;
  }
  return it;
}

std::vector<Client>::iterator Channel::endUsers() { return _Users.end(); }

std::vector<Client>::iterator Channel::beginUsers() { return _Users.begin(); }

// DEBUG
void Channel::printUsers() {
  for (std::vector<Client>::iterator it = _Users.begin(); it != _Users.end();
       it++) {
    std::cout << "Client: " << it->getNick()
              << " SIZE: " << it->getNick().size() << std::endl;
    std::cout << "Is OP? " << it->isOP() << std::endl;
  }
}

void  Channel::removeUser(int value) {
  _Users.erase(searchClient(value));
}
