#include "Channel.hpp"

#include <algorithm>
#include <string>
#include <utility>

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

const std::vector<std::pair<std::string, bool> >& Channel::getUserOn() const {
  return _users;
}

void Channel::addUser(Client client, bool isOp) {
  if (std::find(_users.begin(), _users.end(), client.getUser()) == _users.end())
    _users.push_back(std::make_pair(client.getUser(), isOp));
}

void Channel::rmUser(Client client) {
  std::vector<Client>::iterator it = find(_Users.begin(), _Users.end(), client);
  if (it != _Users.end()) _Users.erase(it);
}

const std::vector<Client>& Channel::getOPsOn() const { return _OPs; }

void Channel::addOP(Client client) {
  _Users.push_back(client.getUser());
  client.setOp(true);
}

void Channel::rmUser(Client client) {
  std::vector<Client>::iterator it = find(_users.begin(), _users.end(), client);
  if (it != _users.end()) _users.erase(it);
  client.setOp(false);
}
