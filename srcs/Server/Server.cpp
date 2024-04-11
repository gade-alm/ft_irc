#include "Server.hpp"

//static void parserTest ( std::string buffer, int i );
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
	if (select(maxfds + 1, &_selectfds, NULL, NULL, NULL) == -1) {
		//perror ("select error");
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
				Client client(_clientfd);
				_Clients.push_back(client);
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
				std::vector<Client>::iterator it = searchClient(i);
				if (it != _Clients.end()) {
					Client& client = *it;
					if(!client.getAuth())
						client.authenticateClient(_password, buf, _Clients);
					if(!client.getAuth()){
						disconnectClient(it);
						return;
					}
				} else {
					disconnectClient(it);
					return;
				}
				std::string buffer(buf, numbytes);
				//parserTest(buffer, i);
				for ( int j = 0; j < maxfds; j++ ) 
				{
					if (FD_ISSET(j, &_masterfds))
						if (j != _serverSocket && j != i)
							sendMessage(buf, j);
				}
				//commandos
			}

		}
	}
}

/* static void parserTest ( std::string buffer, int i ) 
{
	std::cout << "msg from client " << i << ": " << buffer << std::endl;
	return ;
} */

std::vector<Client>::iterator Server::searchClient(int fd){
	std::vector<Client>::iterator it;
	for (it = _Clients.begin(); it != _Clients.end(); ++it) {
    	if (it->getFD() == fd)
       		break;
	}
	return it;
}

void Server::disconnectClient(std::vector<Client>::iterator it){
	Client& client = *it;
	FD_CLR(client.getFD(), &_masterfds);
	std::string message = "You have been disconnected.";
    sendMessage(message, client.getFD());
    close(client.getFD());
	_Clients.erase(it);
}