#include "Client.hpp"

Client::Client() : _nickname(""), _username(""), _operator(false){
}

Client::~Client(){
}

Client::Client( const Client& ){
}

Client& Client::operator=( const Client & ){
    return *this;
}

const std::string Client::getNick() const {
    return _nickname;
}
void Client::setNick(std::string Nick){
    _nickname = Nick;
}

const std::string Client::getUser() const {
    return _username;
}

void Client::setUser(std::string User){
    _username = User;
}

int Client::getFD() const{
    return _clientfd; 
}

void Client::setFD(int FD){
    _clientfd = FD;
}

bool Client::isOP()
 const {
    return _operator;
}

void Client::setOp(bool op){
    _operator = op;
}

void Client::connect(std::string password){

    char		buffer[1024];
	ssize_t		n = recv(_clientfd, buffer, sizeof(buffer) - 1, 0);
    std::cout << buffer << std::endl;
    if (n == -1)
		throw(std::runtime_error("Error. Failed in rcv."));
	else if (n == 0){
        std::cout << "Client Disconected";
		return ;	
    }
	buffer[n] = 0;
    std::string input = buffer;
    checkPass(password, input);
}

void Client::checkPass(std::string password, std::string input){
    std::string message;
    size_t found = input.find("PASS");
    if (found == std::string::npos){
        message = "Server needs a password try to login with one.";
        sendMessage(message, _clientfd);
        disconnect();
        return;
    }
    size_t end = input.find('\n', found + 4);
    std::string afterPass = input.substr(found + 5, end - (found + 5) - 1);
    if (!afterPass.empty() && afterPass[0] == ':')
        afterPass.erase(0, 1);
    if (afterPass != password){
        message = "Wrong password.";
        sendMessage(message, _clientfd);
        disconnect();
        return;
    }
    message = "You are authenticated. Welcome.";
    sendMessage(message, _clientfd);
    std::string channel = "#lobby";
    message = "JOIN " + channel;
    sendMessage(message, _clientfd);
}

void Client::disconnect(){
    std::string message = "You have been disconnected.";
    sendMessage(message, _clientfd);
    if (close(_clientfd) == -1)
        perror("close");
}

//message need \n at the end.
void sendMessage(std::string string, int fd){
    string += "\r\n";
    if(send(fd, string.c_str(), string.size(), 0) == -1)
        perror("send");
}