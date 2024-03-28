#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <sys/socket.h>
# include <sys/types.h>
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
		int	_serverSocket;
		int	_bindSocket;
		int	_listenSocket;
		char**	channel;
	public:
		sockaddr_in serverAddr;
		Server();
		~Server();
		void	setPort ( int portNumber );
		void	setSocket ( int socketFd );
		void	setBind( void );
		void	bindSockets ( void );
		void	listenSockets( void );
		int		getPort ( void );
		int		getSocket ( void );
		int		getBind( void );
		int		getListen( void );

};

#endif
