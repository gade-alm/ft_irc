#include "includes/Server.hpp"

int main ( int ac, char **av ) {

	Server serverClass;

	if (ac != 3) {
		std::cout << "Wrong number of parameters" << std::endl;
		return 1;
	}
	long port = atol(av[1]);
	if (port < MINPORT || port > MAXPORT || port > INT_MAX || port < INT_MIN)
		std::cerr << "Wrong port input" << std::endl;
	serverClass.setPort(port);

	serverClass.setSocket(socket(AF_INET, SOCK_STREAM, 0));
	if (serverClass.getSocket() == -1) {
		perror("Error while opening sockets");
		return (1);
	}

	serverClass.bindSockets();
	if ( serverClass.getBind() == -1) {
		perror("Error while binding");
		return (1);
	}

	serverClass.listenSockets();
	if (serverClass.getListen() == -1) {
		perror("Error while opening sockets");
		return (1);
	}
}
