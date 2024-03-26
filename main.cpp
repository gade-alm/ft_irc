#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <arpa/inet.h>

struct sockaddr_in myaddr;

int main ( int ac, char **av ) {

	if (ac != 3) {
		std::cout << "Wrong number of parameters" << std::endl;
		return 1;
	}
	int port = atoi(av[1]);
	if (port < 1024 || port > 65535)
		std::cerr << "Wrong port input" << std::endl;

	int socketValue = socket(AF_INET, SOCK_STREAM, 0);
	if (socketValue == -1) {
		perror("Error while opening sockets");
		exit (1);
	}

	myaddr.sin_port = htons(port); //Port that was on argv[1]
	myaddr.sin_addr.s_addr = inet_addr("192.168.0.1"); //Chosen ip to connect

	int bindValue = bind(socketValue, (struct sockaddr *)&myaddr, sizeof(struct sockaddr));
	if (bindValue == -1) {
		perror("Error while binding");
		exit (1);
	}
	int listenValue = listen(socketValue, 10);
	if (listenValue == -1) {
		perror("Error while opening sockets");
		exit (1);
	}
}
