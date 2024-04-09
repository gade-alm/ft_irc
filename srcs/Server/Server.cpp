#include "Server.hpp"

static void parserTest ( std::string buffer );
Server::Server( void ) {
}

Server::Server( const Server& ){
}

Server::~Server( void ) {
}

Server::Server( const char* portValue, const std::string &passwordValue ){

	_clientfd = 0;
	maxfds = 0;
	int used = 1;
	int port = atoi(portValue);
	if (port < MINPORT || port > MAXPORT) {
		std::cout << ("Wrong number on port") << std::endl;
		return ;
	}
	_serverPort = port;
	_password = passwordValue;
	initAddr();
	setSocket(socket(serverAddr.sin_family, SOCK_STREAM, PROTOCOL));
	fcntl(_serverSocket, F_SETFL, O_NONBLOCK);


	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &used, sizeof(used)) == -1){
		perror ("setsockopt");
		return ;
	}
	
	setBind();

	listenSockets();

}

void	Server::setSocket( int socketFD ) {
	_serverSocket = socketFD;
	if (_serverSocket == -1 ){
		perror("setSocket");
		return ;
	}
}

void	Server::setBind( void ) {
	_bindSocket =  bind(_serverSocket, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr));
	if (_bindSocket == -1) {
		perror("setBind");
		exit (1);
	}
}

void	Server::initAddr( void ) {
	memset(&(serverAddr.sin_zero), 0, sizeof(serverAddr.sin_zero));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(_serverPort);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	if (serverAddr.sin_addr.s_addr != 0) {
		perror("initAddr");
		return ;
	}
}

void	Server::listenSockets( void ) {
	_listenSocket = listen(_serverSocket, BACKLOG);
	if (_listenSocket == -1) {
		perror("Error while opening sockets");
		return ;
	}
}

int		Server::getSocket( void ) {
	return _serverSocket;
}

void	Server::prepareFDs( void ) {

	FD_ZERO(&_selectfds);
	FD_ZERO(&_masterfds);

	FD_SET(_serverSocket, &_masterfds);
	maxfds = _serverSocket;
}

void	Server::selectLoop( int i, struct sockaddr_in _clientaddr, int numbytes ) {

	_selectfds = _masterfds;
	if (select(maxfds + 1, &_selectfds, NULL, NULL, NULL) == -1) 
	{
		perror ("select error");
		return ;
	}
	if ( FD_ISSET( i , &_selectfds ) != 0 ) 
	{
		if ( i == _serverSocket )
		{
			socklen_t addrSize = sizeof(_clientaddr);
			if ((_clientfd = accept(_serverSocket, (struct sockaddr *)&_clientaddr, &addrSize)) != -1)
			{
				FD_SET(_clientfd, &_masterfds);
				if (_clientfd > maxfds)
					maxfds = _clientfd;
			}
			else
				perror ("accept error");
		}
		else {
			if ((numbytes = recv(i, buf, sizeof(buf), 0)) < 1)
			{
				perror("recv error");
				close (i);
				FD_CLR(i, &_masterfds);
			}
			else
			{
				std::string buffer(buf, numbytes);
				parserTest(buffer);
				for ( int j = 0; j < maxfds; j++ ) 
				{
					if (FD_ISSET(j, &_masterfds))
					{
						if (j != _serverSocket && j != i)
							if (send(j, buf, numbytes, 0) == -1)
								perror ("send");
					}
				}
			}

		}
	}
}

static void parserTest ( std::string buffer ) 
{
	std::string name = buffer.substr(buffer.find("NICK ") + 1);
	std::string user = buffer.substr(buffer.find("USER ") + 1);
	std::cout << name << " " << user << std::endl;
	return ;
}