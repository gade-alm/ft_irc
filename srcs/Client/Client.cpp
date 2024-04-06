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
const std::string Client::getUser() const {
    return _username;
}

bool Client::isOP() const {
    return _operator;
}

void Client::setOp(bool op){
    _operator = op;
}