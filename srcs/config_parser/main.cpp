#include "common.hpp"
#include "ReadConfig.hpp"
#include "ReadConfig.hpp"
#include "Server.hpp"

void handleSigint(std::vector<Server> servers) {
    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
		it->handleSigint();
    exit(EXIT_SUCCESS);
}

int main (int argc, char** argv){

	if (argc < 2) {
		std::cerr << "Missing config file as argument" << std::endl;
		return (1);
	}
	if (argc > 2){
		std::cerr << COLOR_ERROR << "Error: More than one argument." << COLOR_STANDARD << std::endl;
		return (1);
	}
	signal(SIGPIPE, signal_handler);
    signal(SIGINT, signal_handler);

	try {
		std::deque<std::string> tokens = readFile2Buffer(argv[1]);
		std::vector<ServerConfig> configs = parseConfig(tokens);

		//set up servers
		std::vector<Server> servers;
		for (unsigned long i = 0; i < configs.size(); ++i) {
			try {
				Server serv(configs[i]);
				servers.push_back(serv);
				std::cout << "Created server with host: " << configs[i].getHost() << std::endl;
			} catch (const std::exception& e) {
				std::cout << "Failed to create server with host: " << configs[i].getHost() << "\n" << e.what() << std::endl;
			}
		}
		while (true) {
			for (size_t i = 0; i < servers.size(); ++i) {
				if (g_signal_received == SIGINT)
					handleSigint(servers);
				servers[i].serverRun();
			}
		}
	} catch (const std::exception& e) {
		std::cerr << COLOR_ERROR << "Error: " << e.what() << COLOR_STANDARD << std::endl;
		return (1);
	}

	return (0);
}