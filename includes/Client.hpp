#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <netinet/in.h>
# include <cstdio>
# include <unistd.h>

class Client{
	private:
		Client& operator=(const Client & );
		std::string	_nickname;
		std::string	_username;
		bool		_operator;
		int			_clientfd;
		void 		checkPass(std::string password, std::string input);

	public:
		sockaddr_in	_client_address;
		Client();
		Client( const Client& );
		~Client();
		const std::string getNick() const;
		void setNick(std::string Nick);
		const std::string getUser() const;
		void setUser(std::string User);
		int getFD() const;
		void setFD(int FD);
		bool isOP() const;
		void setOp(bool op);
		void connect(std::string password);
		void disconnect();

};

void sendMessage(std::string string, int fd);

#endif