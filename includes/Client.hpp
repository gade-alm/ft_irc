#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Server.hpp"


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
		void setNickname( std::string nick );
		void setUsername( std::string user );
		std::string getNickname( void );
		std::string getUsername( void );

		sockaddr_in	clientAddr;
};

#endif