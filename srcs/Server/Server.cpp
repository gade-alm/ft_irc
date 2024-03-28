#include "Server.hpp"

Server::Server( void ) {
}

Server::Server( const Server& ){
}

Server::~Server( void ) {
}

Server::Server( const char* portValue, const std::string &passwordValue ){
	(void)passwordValue;
	int port = atoi(portValue);
	if (port < MINPORT || port > MAXPORT || port > INT_MAX || port < INT_MIN) {
		perror("Error while opening sockets");
		return ;
	}
	_serverPort = port;
	setSocket(socket(AF_INET,SOCK_STREAM,0));
	if (getSocket() == -1 ){
		perror("Error while binding");
		return ;
	}
	listenSockets();
	if (getListen() == -1) {
		perror("Error while opening sockets");
		return ;
	}
}

void	Server::setPort( int portNumber ) {
	serverAddr.sin_port = portNumber;
}

int		Server::getPort( void ) {
	return serverAddr.sin_port;
}

void	Server::setSocket( int socketFD ) {
	_serverSocket = socketFD;
}

int		Server::getSocket( void ) {
	return _serverSocket;
}

void	Server::setBind( void ) {
	_bindSocket =  bind(getSocket(), (struct sockaddr *)&serverAddr, sizeof(struct sockaddr));
}

int	Server::getBind( void ) {
	return _bindSocket;
}

void	Server::bindSockets( void ) {
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(getPort());
	serverAddr.sin_addr.s_addr = inet_addr(IP);
	memset(&(serverAddr.sin_zero), '\0', 8);
	setBind();
}

void	Server::listenSockets( void ) {
	_listenSocket = listen(getSocket(), BACKLOG);
}

int		Server::getListen( void ) {
	return _listenSocket;
}
