#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <sys/socket.h>
# include <sys/types.h>
# include <netdb.h>
# include <netinet/in.h>
# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <arpa/inet.h>
# include <climits>

# define MINPORT 1024
# define MAXPORT 65535
# define BACKLOG 10
# define IP "127.0.0.1"
# define BOSS "SOU MAQUINA"

class Server {
	private:
		Server( const Server& );
		Server();

		int	_serverSocket;
		int	_listenSocket;
		int	_bindSocket;
		int	_acceptFD;

		const char*	_serverPort;
		std::string _password;

	public:
		struct addrinfo serverAddr;
		struct addrinfo *serverInfo;
		Server( const char* portValue, const std::string &passwordValue );

		void	setSocket ( int socketFd );
		void	setBind( void );
		void	initAddr ( void );
		void	listenSockets( void );
		void	setPassword( std::string pass );
		int		getSocket ( void );
		int		acceptFD( void );

		~Server();
};

#endif
