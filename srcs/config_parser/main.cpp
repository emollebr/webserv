#include "ReadConfig.hpp"
#include "Server.hpp"

int main (int argc, char** argv){

	if (argc < 2)
		return (1);

	readFile2Buffer(argv[1]);
	Server serv;
	return (0);
}