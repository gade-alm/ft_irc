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
	char		buffer[1024];


	server.setFDPoll(24);
	//int num_events = poll(server.pollfds, 0, 30000);
	while (1) {
		sinSize = sizeof(struct sockaddr_in);
		clientfd = accept(server.getSocket(), (struct sockaddr *)&clientaddr, &sinSize);
		if (clientfd > 0){
			int			fd = clientfd;
			ssize_t		n = recv(fd, buffer, sizeof(buffer) - 1, 0);
			buffer[n] = 0;
			std::cout << "Received:" << buffer << "\n";

		}
		/* if (clientfd == -1) {
			std::cerr << "Error accepting client connection" << std::endl;
			continue;
		} */

		/* if (n == -1)
		{
			throw(std::runtime_error("Error. Failed in rcv."));
		}
		else if (n == 0) // recv returns 0 if client disconnects
		{
			return 0;
		} */
	/* if (n > 0 && buffer[n - 1] == '\n')
	{
		buffer[n - 1] = 0;
		client.add_to_cmd(static_cast<std::string>(buffer));
		strncpy(buffer, client.get_cmd().c_str(), BUFFER_READ_SIZE);
		client.clear_cmd();
	}
	else if (n > 0 &	& buffer[n - 1] != '\n')
	{
		client.add_to_cmd(static_cast<std::string>(buffer));
		return 0;
	}
	if (n > 1 && buffer[n - 2] == '\r')
		buffer[n - 2] = 0; */
	}
}