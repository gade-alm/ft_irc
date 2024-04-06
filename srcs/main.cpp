#include "Server.hpp"

int main ( int ac, char **av ) {

	if (ac != 3) {
		std::cout << "Wrong number of parameters" << std::endl;
		return 1;
	}
	int maxfds;

	fd_set selectfds;
	fd_set masterselect;
	FD_ZERO(&selectfds);
	FD_ZERO(&masterselect);

	Server server(av[1], av[2]);
	socklen_t sinSize;
	struct sockaddr_in clientaddr;
	int clientfd = 0;

	FD_SET(server.getSocket(), &masterselect);
	maxfds = server.getSocket();

	while (1) {
		selectfds = masterselect;
		if (select(maxfds + 1, &selectfds, NULL, NULL, NULL) != -1) {
			perror ("select error");
			return 1;
		}
		for ( int i = 0; i <= maxfds; i++ ) {
			if (FD_ISSET(i, &selectfds)) {
				if (i == server.getSocket()) {
					sinSize = sizeof(clientaddr);
					if ((clientfd = accept(server.getSocket(), (struct sockaddr *)&clientaddr, &sinSize)) != -1 ) {
						FD_SET(clientfd, &masterselect);
						if (clientfd > maxfds)
							maxfds = clientfd;
			} else {
				FD_SET(clientfd, &masterselect);
				if (clientfd > maxfds)
					maxfds = clientfd;
			}
		}
		}	
	}
	// epoll_create(2);
	// std::cout << epoll_create(2) << std::endl;
}
}