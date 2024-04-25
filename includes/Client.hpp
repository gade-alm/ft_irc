#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <netinet/in.h>
# include <cstdio>
# include <unistd.h>
# include <vector>

class Client{
	private:
		std::string	_nickname;
		std::string	_username;
		bool		_operator;
		int			_clientfd;
		bool		_authenticated;
		bool		_registred;
		bool 		checkPass(std::string password, std::string input);
		bool checkName(std::string input);
		bool checkNick(std::string input, std::vector<Client> &Clients);

	public:
		sockaddr_in	_client_address;
		Client();
		Client(int fd);
		Client( const Client& copy);
		~Client();
		Client& operator=(const Client & copy);
		bool operator==(const Client & copy) const;
		void setAuth(bool auth);
		bool getAuth();
		void setReg(bool reg);
		bool getReg();
		const std::string getNick() const;
		void setNick(std::string Nick);
		const std::string getUser() const;
		void setUser(std::string User);
		int getFD() const;
		void setFD(int FD);
		bool isOP() const;
		void setOp(bool op);
		void authenticateClient(std::string password, std::string input, std::vector<Client> &Clients);
		void connect(std::string password);
		void disconnect();

};

void sendMessage(std::string string, int fd);

#endif