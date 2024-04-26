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
				std::string buffer(buf, numbytes);
				std::vector<Client>::iterator it = searchClient(i);
				if (it != _Clients.end()) {
					Client& client = *it;
					if(!client.authenticateClient(_password, buf, _Clients)){
						disconnectClient(it);
						return;
					}
					//std::cout << "CLIENTFD: " << client.getFD() << " is Auth: " << client.getAuth() << std::endl;
					cmdHandler(buffer, client);
					
				} else {
					disconnectClient(it);
					return;
				}
			}

		}
	}
	memset(buf, 0, sizeof(buf));
}

std::vector<Client>::iterator Server::searchClient(int fd){
	std::vector<Client>::iterator it;
	for (it = _Clients.begin(); it != _Clients.end(); ++it) {
    	if (it->getFD() == fd)
       		break;
	}
	return it;
}

std::vector<Channel>::iterator Server::searchChannel(std::string channelname){
	std::vector<Channel>::iterator it;
	for (it = _Channels.begin(); it != _Channels.end(); ++it) {
    	if (it->getName() == channelname)
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


void Server::cmdHandler(std::string buffer, Client &client){
	//std::cout << buffer << std::endl;
	std::string cmd = buffer.substr(0, buffer.find(" "));
	void (Server::*myCMDS[4])(std::string, Client&) = {&Server::joinChannel, &Server::quitServer \
	, &Server::deliveryMSG, &Server::kickFromChannel};
	long unsigned int index;
	std::string cmds[4] = {"JOIN", "QUIT", "PRIVMSG", "KICK"};

	for (index = 0; index < sizeof(cmds)/sizeof(cmds[0]); index++){
		if (cmd == cmds[index])
			break;
	}
	if (index < sizeof(cmds)/sizeof(cmds[0]))
		(this->*myCMDS[index])(buffer, client);
}

void Server::joinChannel(std::string buffer, Client &client){
    std::string channelname = buffer.substr(buffer.find(" ") + 1, (buffer.find("\r") - buffer.find("#")));
    if (channelname[0] != '#') 
   		channelname = "#" + channelname;
	channelPrep(channelname, client);
	std::string output = ":" + client.getNick() + "!" + client.getUser() + " JOIN " + channelname;
	sendMessage(output, client.getFD());
}

void Server::quitServer(std::string buffer, Client &client){
	(void)buffer;
	std::vector<Client>::iterator it = searchClient(client.getFD());
	//Missing Channels disconnect.
	disconnectClient(it);
}

void	Server::channelPrep(std::string channelname, Client &client){
	
	std::vector<Channel>::iterator itChannel = searchChannel(channelname);
	if (itChannel != _Channels.end()){
		if (itChannel->searchClient(client.getFD()) == itChannel->endUsers())
			itChannel->addUser(client);
		itChannel->printUsers();
		return ;
	}
	Channel channel(channelname);
	channel.addUser(client);
	std::vector<Client>::iterator itClient = channel.searchClient(client.getFD());
	itClient->setOp(true);
	_Channels.push_back(channel);
	
	itChannel = searchChannel(channelname);
	//itChannel->printUsers();
}

void	Server::deliveryMSG(std::string buffer, Client &client){
	size_t start = buffer.find("#");
	size_t end = buffer.find(" ", start);

	std::string channelname = buffer.substr(start, (end - start));
	//std::cout << "CHANNEL NAME: " << channelname << std::endl;
	start = buffer.find(":", end + 1);
	std::string message = ":" + client.getNick() + "!~" + client.getUser() + " PRIVMSG "\
	+ channelname + " " + buffer.substr(start);
	//std::cout << "MESSAGE: " << message << std::endl;
	std::vector<Channel>::iterator itChannel = searchChannel(channelname);
	//std::cout << itChannel->getName() << std::endl;

	for(std::vector<Client>::iterator itClient = itChannel->beginUsers(); itClient != itChannel->endUsers(); itClient++){
		if(itClient->getFD() != client.getFD())
			sendMessage(message, itClient->getFD());
	}
}

void	Server::kickFromChannel(std::string buffer, Client &client){
	size_t start = buffer.find("#");
	size_t end = buffer.find(" ", start);
	std::string nickname;
	std::string channelname = buffer.substr(start, end - start);
	std::vector<Channel>::iterator it = searchChannel(channelname);
	//std::cout << "CHANNELNAME: " << channelname << " SIZE: " << channelname.size() << std::endl;
	start = end + 1;
	end = buffer.find(" ", start);
	std::string nick;
	std::string cmd;
	if (end == std::string::npos){
		end = buffer.find("\r", start);
		nick = buffer.substr(start, end - start);
		std::cout << "NICK: " << nick << " SIZE: " << nick.size() << std::endl;
		if (!it->searchClient(client.getNick())->isOP()){
			sendMessage("You dont have the rights to kick Users.", client.getFD());
			return;
		}
		if(it->searchClient(nick) == it->endUsers()){
			sendMessage("User not found.", client.getFD());
			return;
		}
		cmd = ":" + client.getNick() + "!" + client.getUser()\
		+ " KICK " + channelname + " " + nick + "\r\n";
		for(std::vector<Client>::iterator itClient = it->beginUsers(); itClient != it->endUsers(); itClient++){
			sendMessage(cmd, itClient->getFD());
		}
		it->rmUser(*it->searchClient(nick));
		return;
	}
	/* nick = buffer.substr(start, end - start);
	std::cout << "NICK: " << nick << " SIZE: " << nick.size() << std::endl;
	start = end + 1;
	end = buffer.find("\r", start);
	std::string reason = buffer.substr(start, end - start);
	if (!it->searchClient(client.getNick())->isOP()){
		sendMessage("You dont have the rights to kick Users.", client.getFD());
		return;
	}
	if(!it->rmUser(nick)){
		sendMessage("User not found.", client.getFD());
		return;
	}
	cmd = "KICK " + channelname + " " + nick + " " + reason + "\r\n";
	sendMessage(cmd, client.getFD()); */
	// KICK #channelname username reason
	// :server_name KICK #channelname username :reason

}