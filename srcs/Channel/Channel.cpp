#include "Channel.hpp"

Channel::Channel() : _name(""), _password("") {}

Channel::Channel(std::string name, std::string password)
    : _name(name), _password(password) {}

Channel::Channel(std::string name) : _name(name), _password("") {}

Channel::~Channel() {}

Channel::Channel(const Channel&) {}

Channel& Channel::operator=(const Channel&) { return *this; }

bool Channel::operator==(Channel const& value) { return value._name == _name; }

void Channel::setName(std::string name) { _name = name; }

const std::string Channel::getName() const { return _name; }

void Channel::setPassword(std::string password) { _password = password; }

const std::string Channel::getPassword() const { return _password; }

void Channel::setTopic(std::string topic) { _topic = topic; }

const std::string Channel::getTopic() const { return _topic; }

void Channel::setInvMode(bool mode) { _inviteonly = mode; }

const bool Channel::getInvMode() const { return _inviteonly; }

void Channel::setLimit(long limit) { _limit = _userlimit; }

const long Channel::getLimit() const { return _userlimit; }

void Channel::setLimitMode(bool mode) { _limit = mode; }

const bool Channel::getInvMode() const { return _limit; }

const std::vector<Client>& Channel::getUserOn() const { return _Users; }

void Channel::addUser(Client client) { _Users.push_back(client); }

void Channel::rmUser(Client client) {
  std::vector<Client>::iterator it = find(_Users.begin(), _Users.end(), client);
  if (it != _Users.end()) _Users.erase(it);
}

const std::vector<Client>& Channel::getOPsOn() const { return _OPs; }

void Channel::addOP(Client client) {
  _Users.push_back(client);
  client.setOp(true);
}

void Channel::rmUser(Client client) {
  std::vector<Client>::iterator it = find(_OPs.begin(), _OPs.end(), client);
  if (it != _OPs.end()) _OPs.erase(it);
  client.setOp(false);
}
