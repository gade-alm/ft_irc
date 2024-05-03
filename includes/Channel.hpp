#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <algorithm>
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
  Channel& operator=(const Channel& copy);

  void setName(std::string name);
  const std::string getName() const;

  void setPassword(std::string password);
  const std::string getPassword() const;

  void setTopic(std::string topic);
  const std::string getTopic() const;

  void setTopicMode(bool topic);
  bool getTopicMode() const;

  const std::vector<Client>& getUserOn() const;
  void addUser(Client& client);
  void rmUser(Client& client);

  void setInvMode(bool mode);
  bool getInvMode() const;

  void setLimit(long limit);
  long getLimit() const;

  void setLimitMode(bool mode);
  bool getLimitMode() const;

  std::vector<Client>::iterator endUsers();
  std::vector<Client>::iterator beginUsers();

  std::vector<Client>::iterator searchClient(int fd);
  std::vector<Client>::iterator searchClient(std::string name);

  std::vector<int> _invitation;

  void printUsers();
};

#endif
