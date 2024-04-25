#include "ReadConfig.hpp"
#include "Server.hpp"

int main (int argc, char** argv){

	

	if (argc < 2)
		return (1);

	//RESTRUCTURE: 1. Read2Buff; 2. Parse Server; 3. Profit
	std::vector<ServerConfig> myserver = readFile2Buffer(argv[1]);
	
	for (std::vector<ServerConfig>::iterator it = myserver.begin(); it < myserver.end(); it++)
		std::cout << *it << std::endl;

	// START SERVER
	// Server serv;
	return (0);
}