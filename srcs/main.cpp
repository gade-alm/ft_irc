#include "Server.hpp"

int main ( int ac, char **av ) {

	if (ac != 3) {
		std::cout << "Wrong number of parameters" << std::endl;
		return 1;
	}
	Server(av[1], av[2]);
	// epoll_create(2);
	// std::cout << epoll_create(2) << std::endl;
}
