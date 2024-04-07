#include "Server.hpp"

int main ( int ac, char **av ) {

	if (ac != 3) {
		std::cout << "Wrong number of parameters" << std::endl;
		return 1;
	}
	int maxfds;
	int nbytes;
	char buf[512];

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
		if (select(maxfds + 1, &selectfds, NULL, NULL, NULL) == -1) {
			perror ("select error");
			break ;
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
				perror ("accept error ");
			}
		}
		else {
			if ((nbytes = recv(i, buf,sizeof(buf), 0)) < 1) {
					perror("receive");
				close(i);
				FD_CLR(i, &masterselect);
		}
		else {
			for (int j = 0; j < maxfds; j++) {
				if (FD_ISSET(j, &masterselect)) {
					if (j != server.getSocket() && j != i) {
						if (send(j, buf, nbytes, 0) == -1)
							perror("send");
					}
				}
			}
		}
		}	
		// std::cout << "comandos: " << buf << std::endl;
	}
}
}
}