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

void	Server::selectLoop( struct sockaddr_in _clientaddr ) {

	_selectfds = _masterfds;
	if (select(maxfds + 1, &_selectfds, NULL, NULL, NULL) == -1) {
		perror ("SELECT:");
		return ;
	}
	for (int i = 0; i <= maxfds; i++) {
		
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
			int numbytes = 0;
			if ((numbytes = recv(i, buf, sizeof(buf), MSG_DONTWAIT)) < 1)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					continue;
				else
				{
					perror("recv error");
					close (i);
					FD_CLR(i, &_masterfds);
				}
			}
			else
			{
				std::string buffer(buf, numbytes);
				for (int j = 0; j < maxfds; j++){
					if (FD_ISSET(j, &_masterfds)){
					std::vector<Client>::iterator it = searchClient(j + 1);
					if (it != _Clients.end()) {
						Client& client = *it;
						client.authenticateClient(_password, buf, _Clients);
						std::cout << "CLIENTFD: " << client.getFD() << " is Auth: " << client.getAuth() << std::endl;
						if (client.getAuth())
							cmdHandler(buffer, client);
						} else {
							disconnectClient(it);
							return;
						}
					}
					}
				}
			}
		}
	}
}

std::vector<Client>::iterator Server::searchClient(int fd){
	std::vector<Client>::iterator it;
	for (it = _Clients.begin(); it != _Clients.end(); it++) {
    	if (it->getFD() == fd)
       		break;
	}
	return it;
}

std::vector<Client>::iterator Server::searchClient(std::string name){
	std::vector<Client>::iterator it;
	for (it = _Clients.begin(); it != _Clients.end(); it++) {
    	if (it->getNick() == name)
       		break;
	}
	return it;
}


std::vector<Channel>::iterator Server::searchChannel(std::string channelname){
	std::vector<Channel>::iterator it;
	for (it = _Channels.begin(); it != _Channels.end(); ++it) {
		//std::cout << "ClientName: " << it->getName() << " SIZE: "<< it->getName().size() << std::endl;
    	//std::cout << "Name: " << channelname << " SIZE: "<< channelname.size() << std::endl;
    	if (it->getName() == channelname){
			//std::cout << "TRUE CHANNEL" << std::endl;
       		break;
		}
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
	std::vector<std::string> CMD = parseCMD(buffer);
	std::string cmd = buffer.substr(0, buffer.find(" "));
	void (Server::*myCMDS[5])(std::vector<std::string>, Client&) = {&Server::joinChannel, &Server::quitServer \
	, &Server::deliveryMSG, &Server::kickFromChannel, &Server::topicChannel};
	long unsigned int index;
	std::string cmds[5] = {"JOIN", "QUIT", "PRIVMSG", "KICK", "TOPIC"};

	for (index = 0; index < sizeof(cmds)/sizeof(cmds[0]); index++){
		if (cmd == cmds[index])
			break;
	}
	if (index < sizeof(cmds)/sizeof(cmds[0]))
		(this->*myCMDS[index])(CMD, client);
}

void Server::joinChannel(std::vector<std::string> CMD, Client &client){
    std::string channelname = CMD[1];
    if (channelname[0] != '#') 
   		channelname = "#" + channelname;
	channelPrep(channelname, client);
	std::string output = ":" + client.getNick() + "!" + client.getUser() + " JOIN " + channelname;
	sendMessage(output, client.getFD());

	std::string message = ":" + client.getNick() + "!" + client.getUser() + " JOIN " + channelname + "\r\n";
	std::vector<Channel>::iterator itChannel = searchChannel(channelname);
	for(std::vector<Client>::iterator itClient = itChannel->beginUsers(); itClient != itChannel->endUsers(); itClient++){
		if(itClient->getFD() != client.getFD())
			sendMessage(message, itClient->getFD());
	}
/* 	std::vector<Channel>::iterator itChannel = searchChannel(channelname);
	itChannel->printUsers(); */
}

void Server::quitServer(std::vector<std::string> CMD, Client &client){
	(void)CMD;
	std::vector<Client>::iterator it = searchClient(client.getFD());
	//Missing Channels disconnect.
	disconnectClient(it);
}

void	Server::channelPrep(std::string channelname, Client &client){
	
	std::vector<Channel>::iterator itChannel = searchChannel(channelname);
	if (itChannel != _Channels.end()){
		if (itChannel->searchClient(client.getFD()) == itChannel->endUsers())
			itChannel->addUser(client);
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

void	Server::deliveryMSG(std::vector<std::string> CMD, Client &client){
	//PRIVMSG Gabriel :oh
	std::string channelname = CMD[1];
	//std::cout << "CHANNEL NAME: " << channelname << std::endl;
	std::string message = ":" + client.getNick() + "!~" + client.getUser() + " PRIVMSG "\
	+ channelname + " " + CMD[2];
	//std::cout << "MESSAGE: " << message << std::endl;
	//std::cout << itChannel->getName() << std::endl;
	if (channelname[0] != '#'){
		std::vector<Client>::iterator itClient = searchClient(channelname);
		sendMessage(message, itClient->getFD());
		return ;
	}

	std::vector<Channel>::iterator itChannel = searchChannel(channelname);
	for(std::vector<Client>::iterator itClient = itChannel->beginUsers(); itClient != itChannel->endUsers(); itClient++){
		if(itClient->getFD() != client.getFD())
			sendMessage(message, itClient->getFD());
	}
}

std::string prepReason(std::vector<std::string> CMD){
	std::string reason;
	for(std::vector<std::string>::iterator it = CMD.begin() + 3; it != CMD.end(); it++){
		if (*it != CMD[3])
			reason += " ";
		reason += *it;
	}
	return reason;

}

void	Server::kickFromChannel(std::vector<std::string> CMD, Client &client){
	// KICK #channelname username reason
	// :server_name KICK #channelname username :reason
	std::string channelname = CMD[1];
	std::string nick = CMD[2];
	std::string reason = (CMD.size() >= 4) ? prepReason(CMD) : "";
	std::vector<Channel>::iterator it = searchChannel(channelname);
	if (it == _Channels.end())
		return;
	if (!it->searchClient(client.getNick())->isOP()){
		sendMessage("You dont have the rights to kick Users.", client.getFD());
		return;
	}
	if(it->searchClient(nick) == it->endUsers()){
		sendMessage("User not found.", client.getFD());
		return;
	}
	std::string cmd = ":" + client.getNick() + "!" + client.getUser()\
	+ " KICK " + channelname + " " + nick + ((reason.empty()) ? "" : (" " + reason)) + "\r\n";
	for(std::vector<Client>::iterator itClient = it->beginUsers(); itClient != it->endUsers(); itClient++){
		sendMessage(cmd, itClient->getFD());
	}
	it->rmUser(*it->searchClient(nick));
	return;

}


std::vector<std::string> Server::parseCMD(std::string buffer){
	size_t start = 0;
	size_t end;
	std::string word;

	std::vector<std::string> CMD;
	//std::cout << "ENTROU" << std::endl;
	while (end != buffer.size() - 2){
		end = buffer.find(" ", start);
		if (end == std::string::npos){
			end = buffer.find("\r", start);
			word = buffer.substr(start, end - start);
			CMD.push_back(word);
			//std::cout << "WORD: " << word << " SIZE " << word.size() << std::endl;
			break ;
		}
		word = buffer.substr(start, end - start);
		CMD.push_back(word);
		//std::cout << "WORD: " << word << " SIZE " << word.size() << std::endl;
		start = end + 1;
	}
	return CMD;
}

void	Server::topicChannel(std::vector<std::string> CMD, Client &client){
	std::string channelName = CMD[1];
    std::vector<Channel>::iterator it = searchChannel(channelName);
	if (CMD.size() == 2){
        if (it != _Channels.end()) {
            std::string topic = it->getTopic();
            std::string msg = ((!topic.empty()) ? ":IRC 332 " : ":IRC 331 ") + client.getNick() + " " + channelName + \
			" :" + ((!topic.empty()) ? topic : "No topic is set")+ "\r\n";
            sendMessage(msg, client.getFD());
        }
		return ;
	}

	if (it->getTopicMode() && !it->searchClient(client.getFD())->isOP()){
		std::string msg = ":IRC 482 " + client.getNick() + " " + channelName + " :You're not channel operator\r\n";
        sendMessage(msg, client.getFD());
		return ;
	}

	//:Nick!User@host TOPIC #channelname :new topic\r\n
	std::string topic;
	for(std::vector<std::string>::iterator itC = CMD.begin() + 2; itC != CMD.end(); itC++){
		if (*itC != CMD[2])
			topic += " ";
		topic += *itC;
	}

	std::string msg = ":" + client.getNick() + "!" + client.getUser() + " TOPIC " + channelName \
	+ " " + topic + "\r\n";
	//std::cout << "TOPIC: " << topic << std::endl;

	it->setTopic(topic.substr(1, topic.size() - 1));
	for(std::vector<Client>::iterator itClient = it->beginUsers(); itClient != it->endUsers(); itClient++){
		sendMessage(msg, itClient->getFD());
	}
}