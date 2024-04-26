#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "Client.hpp"

class Channel {
 private:
  std::string _name;
  std::string _password;
  std::string _topic;
  bool _topicneedop;
  bool _inviteonly;
  bool _limit;
  long _userlimit;
  std::vector<Client> _Users;

 public:
  Channel();
  Channel(std::string name);
  ~Channel();
  Channel(const Channel& copy);

  void setName(std::string name);
  const std::string getName() const;

  void setPassword(std::string password);
  const std::string getPassword() const;
  const std::vector<Client>& getUserOn() const;
  void addUser(Client& client);
  void rmUser(Client& client);

  void setTopic(std::string topic);
  const std::string getTopic() const;

  void addOP(Client client);
  void rmOP(Client client);

  void setInvMode(bool mode);
  const bool getInvMode() const;

  std::vector<Client>::iterator endUsers();
  std::vector<Client>::iterator beginUsers();

  std::vector<Client>::iterator searchClient(int fd);

  void printUsers();

  void setLimit(long limit);
  const long getLimit() const;

  void setLimitMode(bool mode);
  const bool getLimitMode() const;
};
#endif
