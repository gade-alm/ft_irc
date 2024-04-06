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
# include <sys/epoll.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/time.h>
# include <poll.h>


# define MINPORT 1024
# define MAXPORT 65535
# define BACKLOG 10
# define IP "127.0.0.1"
# define BOSS "SOU MAQUINA"
# define PROTOCOL 0
# define MAXUSERS 24

class Server {
	private:
		Server( const Server& );
		Server();

		int	_serverSocket;
		int	_listenSocket;
		int	_bindSocket;
		int	_acceptFD;

		int	_serverPort;
		std::string _password;

	public:
		struct pollfd pollfds[MAXUSERS];
		sockaddr_in serverAddr;
		Server( const char* portValue, const std::string &passwordValue );

		void	setSocket ( int socketFd );
		void	setBind( void );
		void	initAddr ( void );
		void	listenSockets( void );
		int		acceptFD( void );
		int		getSocket( void );
		void	setFDPoll( int i );

		~Server();
};

#endif
