#include "Server.hpp"
#include "Client.hpp"
#include <signal.h>

bool closedServer = false;

void sigHandler(int signal){
	if ( signal == SIGINT ){
		close(4);
 		if (closedServer == false)
    		closedServer = true;
		return ;
	}
}

int main ( int ac, char **av ) {

	if (ac != 3) {
		std::cout << "Wrong number of parameters" << std::endl;
		return 1;
	}
	// struct sigaction sa;
	// sigemptyset(&sa.sa_mask);
	// sa.sa_handler = sigHandler;
	// sigaction(SIGINT, &sa, NULL);
	signal(SIGINT, sigHandler);
	// sa.sa_flags = 0;

	Server server(av[1], av[2]);
	struct sockaddr_in clientaddr;
	server.prepareFDs();

	while (!closedServer) {
		server.selectLoop( clientaddr );
	}
}
