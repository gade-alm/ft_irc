#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>

class Client{
	private:
		Client();
		~Client();
		Client( const Client& );
		Client& operator=(const Client & );
		std::string	_nickname;
		std::string	_username;
		bool		_operator;
	public:
		const std::string Client::getNick() const;
		const std::string Client::getUser() const;
		const bool Client::isOP() const;
		void setOp(bool op);
};

#endif