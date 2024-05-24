#include "Server.hpp"
#include "Client.hpp"
#include <signal.h>


int main ( int ac, char **av ) {

	if (ac != 3) {
		std::cout << "Wrong number of parameters" << std::endl;
		return 1;
	}
	// struct sigaction sa;
	// sa.sa_handler = sigHandler;

	// if (sigaction(SIGINT, &sa, NULL))
	// 	std::cout << " TESTE SIGNAL" << std::endl;
	Server server(av[1], av[2]);
	struct sockaddr_in clientaddr;
	server.prepareFDs();

	while (1) {
		server.selectLoop( clientaddr);
	}
}
