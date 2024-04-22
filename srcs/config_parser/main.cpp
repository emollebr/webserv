#include "ReadConfig.hpp"
#include "Server.hpp"

int main (int argc, char** argv){

	

	if (argc < 2)
		return (1);

	//RESTRUCTURE: 1. Read2Buff; 2. Parse Server; 3. Profit
	ServerConfig myserver = readFile2Buffer(argv[1]);
	
	// START SERVER
	// Server serv;
	return (0);
}