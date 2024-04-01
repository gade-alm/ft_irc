#include "Server.hpp"

Server::Server( void ) {
}

Server::Server( const Server& ){
}

Server::~Server( void ) {
	freeaddrinfo(serverInfo);
}

Server::Server( const char* portValue, const std::string &passwordValue ){

	int port = atoi(portValue);
	if (port < MINPORT || port > MAXPORT || port > INT_MAX || port < INT_MIN) {
		perror("Error while opening sockets");
		return ;
	}
	_serverPort = portValue;
	setPassword(passwordValue);

	initAddr();
	int getAddrstatus = (getaddrinfo(NULL, _serverPort, &serverAddr, &serverInfo));
	if (getAddrstatus != 0) {
		perror ("Error on ADDR Status");
		return ;
	}
	setSocket(socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol));
	if (getSocket() == -1 ){
		perror("Error while binding");
		return ;
	}
	setBind();
	if (_bindSocket == -1) {
		perror("Error while binding socket");
		return ;
	}
	listenSockets();
	if (_listenSocket == -1) {
		perror("Error while opening sockets");
		return ;
	}
}

void	Server::setSocket( int socketFD ) {
	_serverSocket = socketFD;
}

int		Server::getSocket( void ) {
	return _serverSocket;
}

void	Server::setBind( void ) {
	_bindSocket =  bind(getSocket(), serverAddr.ai_addr, serverAddr.ai_addrlen);
}

void	Server::initAddr( void ) {
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.ai_family = AF_INET;
	serverAddr.ai_socktype = SOCK_STREAM;
	serverAddr.ai_flags = AI_PASSIVE;
}

void	Server::listenSockets( void ) {
	_listenSocket = listen(getSocket(), BACKLOG);
}

void	Server::setPassword( std::string pass ){
	_password = pass;
}

int		Server::acceptFD( void ) {

	_accetFD = accept(_serverSocket, )
}