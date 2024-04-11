#include "Server.hpp"
#include "Client.hpp"

int main ( int ac, char **av ) {

	if (ac != 3) {
		std::cout << "Wrong number of parameters" << std::endl;
		return 1;
	}
	int nbytes = 0;


	Server server(av[1], av[2]);
	struct sockaddr_in clientaddr;
	server.prepareFDs();

	while (1) {
		
		for (int i = 0; i <= server.maxfds; i++)
			server.selectLoop(i, clientaddr, nbytes);
	}
}
