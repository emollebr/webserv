#include "common.hpp"
#include "ReadConfig.hpp"
#include "Server.hpp"

void handleSigint(std::vector<Server> servers) {
    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
		it->handleSigint();
    exit(EXIT_SUCCESS);
}

int main (int argc, char** argv){

	if (argc < 2)
		return (1);

	signal(SIGPIPE, signal_handler);
    signal(SIGINT, signal_handler);

	try {
		std::deque<std::string> tokens = readFile2Buffer(argv[1]);
		std::vector<ServerConfig> configs = parseConfig(tokens);

		//set up servers
		std::vector<Server> servers;
		for (unsigned long i = 0; i < configs.size(); ++i) {
			Server serv(configs[i]);
			servers.push_back(serv);
			std::cout << "Server host: " << configs[i].getHost() << std::endl;
		}
		//infinite loop
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