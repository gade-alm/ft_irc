#include "Server.hpp"

int main ( int ac, char **av ) {

	if (ac != 3) {
		std::cout << "Wrong number of parameters" << std::endl;
		return 1;
	}
	Server server(av[1], av[2]);
	socklen_t sinSize;
	struct sockaddr_in clientaddr;
	int clientfd = 0;


	server.setFDPoll(24);
	int num_events = poll(server.pollfds, 0, 30000);
	while (1) {
		sinSize = sizeof(struct sockaddr_in);
		if ((clientfd = accept(server.getSocket(), (struct sockaddr *)&clientaddr, &sinSize)) != -1 ) {
			std::cout << "Clientfd value:" << clientfd << std::endl;
		}
		// close(clientfd);
	}
	// epoll_create(2);
	// std::cout << epoll_create(2) << std::endl;
}