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
	_password = passwordValue;
	initAddr();
	setSocket(socket(serverAddr.sin_family, SOCK_STREAM, PROTOCOL));
	fcntl(_serverSocket, F_SETFL, O_NONBLOCK);

	setBind();

	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &used, sizeof(used)) == -1){
		perror ("setsockopt");
		return ;
	}

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
	std::cout << _serverSocket << std::endl;
	_bindSocket =  bind(_serverSocket, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr));
	std::cout << _bindSocket << std::endl;
	if (_bindSocket == -1) {
		perror("setBind");
		return ;
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
 
void	Server::setFDPoll( int i ) {
	pollfds[i].fd = getSocket();
	pollfds[i].events = POLLIN;
}
// int		Server::acceptFD( void ) {
// 	int newSocket;

// 	newSocket = accept(_serverSocket, /*SOCKET CLIENT, SIZECSOCKETCLIENTE*/);
// 	if (newSocket == -1) {
// 		perror("Error while trying to create new socket to accept");
// 		return 1;
// 	}
// }