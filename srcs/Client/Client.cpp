#include "Client.hpp"

Client::Client(){
}

Client::~Client(){
}

Client::Client( const Client& ){
}

Client& Client::operator=( const Client & ){
	return *this;
}

void	Client::setNickname( std::string nick ) {
	_nickname = nick;
}

std::string Client::getNickname( void ) {
	return _nickname;
}

void Client::setUsername( std::string user ){
	_username = user;
}

std::string Client::getUsername( void ) {
	return _username;
}

sockaddr_in& Client::getClientAddr( void ) {
	return clientAddr;
}