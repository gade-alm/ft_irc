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
  std::vector<Client> _OPs;

 public:
  Channel();
  Channel(std::string name, std::string password);
  Channel(std::string name);
  ~Channel();
  Channel(const Channel&);
  Channel& operator=(const Channel&);
  bool operator==(Channel const& value);

  void setName(std::string name);
  const std::string getName() const;

  void setPassword(std::string password);
  const std::string getPassword() const;

  void setTopic(std::string topic);
  const std::string getTopic() const;

  const std::vector<Client>& getUserOn() const;
  void addUser(Client client);
  void rmUser(Client client);

  const std::vector<Client>& getOPsOn() const;
  void addOP(Client client);
  void rmOP(Client client);

  void setInvMode(bool mode);
  const bool getInvMode() const;

  void setLimit(long limit);
  const long getLimit() const;

  void setLimitMode(bool mode);
  const bool getLimitMode() const;
};
#endif
