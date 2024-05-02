#include "common.hpp"
#include "Server.hpp"

void handleSigint(std::vector<Server> servers) {
    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
		it->handleSigint();
    exit(EXIT_SUCCESS);
}

int main (int argc, char** argv){

	

	if (argc < 2) {
		std::cout << "Missing config file as argument" << std::cout;
		return (1);
	}

	signal(SIGPIPE, signal_handler);
    signal(SIGINT, signal_handler);

	//server setup
	/* std::vector<ServerConfig> configs = readFile2Buffer(argv[1]);
	for (std::vector<ServerConfig>::const_iterator it = configs.begin(); it != configs.end(); it++) {
		try {
			Server serv(*it);
			servers.push_back(serv);
		} catch (const std::exception& e) {
        	std::cout << "Failed to create server" << std::endl;
		}
	} */
	//temporary
	try {
		std::deque<std::string> tokens = readFile2Buffer(argv[1]);
		std::vector<ServerConfig> configs = parseConfig(tokens);
		std::vector<Server> servers;
		for (unsigned long i = 0; i < configs.size(); ++i) {
			std::cout << configs[i] << std::endl << std::endl;
			// Server serv(configs[i]);
			// servers.push_back(serv);
			// std::cout << "Server host: " << configs[i].getHost() << std::endl;
		}
		// while (true) {
		// 	for (size_t i = 0; i < servers.size(); ++i) {
		// 		if (g_signal_received == SIGINT)
		// 			handleSigint(servers);
		// 		servers[i].serverRun();
		// 	}
		// }
	} catch (const std::exception& e) {
		std::cerr << COLOR_ERROR << "Error: " << e.what() << COLOR_STANDARD << std::endl;
		return (1);
	}

	return (0);
}