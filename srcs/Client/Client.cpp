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
    if (n == -1)
		throw(std::runtime_error("Error. Failed in rcv."));
	else if (n == 0){
        std::cout << "Client Disconected";
		return ;	
    }
	buffer[n] = 0;
	//std::cout << "Received:" << buffer;
    //std::cout << "Password Server: " << password << std::endl;
    std::string input = buffer;
    checkPass(password, input);
   

   /*  for (char* it = &CMD[0]; it != &CMD[0] + CMD.size(); ++it) {
    std::cout << *it; */
}

bool Client::checkPass(std::string password, std::string input){
    size_t found = input.find("PASS");
    if (found == std::string::npos){
        std::cout << "Server needs a password try to login with one." << std::endl;
        return false;
    }
    size_t end = input.find('\n', found + 4);
    if (end == std::string::npos) {
        end = input.size();
    }
    std::string afterPass = input.substr(found + 5, end - (found + 5) - 1); 
    if (afterPass != password){
        std::cout << afterPass;
        return false;
    }
    std::cout << "You are authenticated. Welcome." << std::endl;
    return true;
}