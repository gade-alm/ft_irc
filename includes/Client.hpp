#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <netinet/in.h>

class Client{
	private:
		Client& operator=(const Client & );
		std::string	_nickname;
		std::string	_username;
		bool		_operator;
		int			_clientfd;

	public:
		sockaddr_in	_client_address;
		Client();
		Client( const Client& );
		~Client();
		const std::string getNick() const;
		const std::string getUser() const;
		bool isOP() const;
		void setOp(bool op);
};

#endif