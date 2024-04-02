#include "Server.hpp"

Server::Server( void ) {
}

Server::Server( const Server& ){
}

Server::~Server( void ) {
}

Server::Server( const char* portValue, const std::string &passwordValue ){

	int used = 1;
	int port = atoi(portValue);
	if (port < MINPORT || port > MAXPORT) {
		std::cout << ("Wrong number on port") << std::endl;
		return ;
	}
	_serverPort = port;
	setPassword(passwordValue);

	initAddr();
	setSocket(socket(serverAddr.sin_family, SOCK_STREAM, PROTOCOL));

	setBind();
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &used, sizeof(used)) == -1){
		perror ("setsockopt");
		return ;
	}

	listenSockets();
	// acceptFD();
}

void	Server::setSocket( int socketFD ) {
	_serverSocket = socketFD;
	if (_serverSocket == -1 ){
		perror("setSocket");
		return ;
	}
}

void	Server::setBind( void ) {
	_bindSocket =  bind(_serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if (_bindSocket == -1) {
		perror("setBind");
		return ;
	}
}

void	Server::initAddr( void ) {
	memset(&(serverAddr.sin_zero), 0, sizeof(serverAddr.sin_zero));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(_serverPort);
	serverAddr.sin_addr.s_addr = inet_aton("127.0.0.1", &(serverAddr.sin_addr));
	if (serverAddr.sin_addr.s_addr == 0) {
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

void	Server::setPassword( std::string pass ) {
	_password = pass;
}

// int		Server::acceptFD( void ) {
// 	int newSocket;

// 	newSocket = accept(_serverSocket, /*SOCKET CLIENT, SIZECSOCKETCLIENTE*/);
// 	if (newSocket == -1) {
// 		perror("Error while trying to create new socket to accept");
// 		return 1;
// 	}
// }
