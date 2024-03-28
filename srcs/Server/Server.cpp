#include "Server.hpp"

Server::Server( void ) {
}

Server::Server( const Server& ){
}

Server::~Server( void ) {
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
