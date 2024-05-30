#include "Server.hpp"
#include "Client.hpp"
#include <signal.h>

bool closedServer = false;

void sigHandler(int signal){
	(void)signal;
 		if (closedServer == false)
    		closedServer = true;
		return ;
}

int main ( int ac, char **av ) {

	if (ac != 3) {
		std::cout << "Wrong number of parameters" << std::endl;
		return 1;
	}
	signal(SIGINT, sigHandler);
	signal(SIGQUIT, sigHandler);

	Server server(av[1], av[2]);
	struct sockaddr_in clientaddr;
	server.prepareFDs();

	while (!closedServer) {
		server.selectLoop( clientaddr );
	}
	server.closeServer();
}
