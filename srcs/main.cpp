#include "Client.hpp"
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
	Client client;
	


	server.setFDPoll(24);
	//int num_events = poll(server.pollfds, 0, 30000);
	while (1) {
		sinSize = sizeof(struct sockaddr_in);
		clientfd = accept(server.getSocket(), (struct sockaddr *)&clientaddr, &sinSize);
		client.setFD(clientfd);
		if (client.getFD() > 0)
			client.connect(av[2]);
	}
}