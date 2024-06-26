/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReadConfig.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lejimene <lejimene@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/05 10:20:40 by jschott           #+#    #+#             */
/*   Updated: 2024/05/14 15:22:15 by lejimene         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ReadConfig.hpp"

//Print Tokens [HELPER FUNCTION]
void	printTokens(std::deque<std::string> tokens){
	for (tokeniterator it = tokens.begin(); it != tokens.end(); it++)
		std::cout<< *it << std::endl;
}

// finds the closing limiter in a dequeue of tokens to a given opening limiter char
// returns iterator to the position, NULL if limiter is not closed within the dequeue
tokeniterator getClosingBraket (tokeniterator start, tokeniterator end) {
	std::string	open =	"{[(\'\"`";
	std::string	close =	"}])\'\"`";
	std::string	limiter_open = *start;
	std::string	limiter_close;

	//return NULL if limiter_open is unknown
	if (open.find(limiter_open) == open.length())
		return end;
	//set closing limiter
	limiter_close = close[open.find(limiter_open)];
	
	//iterate through dequeue and return closing limiter if found

	std::stack<std::string> stack;
	for (tokeniterator pos = start + 1; pos < end; pos++) {
		if (*pos == limiter_close) {
			if (limiter_open == limiter_close || stack.empty())
				return pos;
			else if (!stack.empty())
				stack.pop();
		}
		else if (*pos == limiter_open)
			stack.push(*pos);
	}
	return end;
}

//Check if all brakets and quotes are closed correctly
bool isBalanced(std::stringstream& ss) {
    std::stack<char> stack;
    char c;
	std::string open =	"{[(\'\"`";
	std::string close = "}])\'\"`";
	
	for (size_t i = 0; i < strlen(open.c_str()); i++) {
		while (ss.get(c)) {
			if (c == open[i]) {
				stack.push(c);  // Push open brace onto the stack
			}
			else if (c == close[i]) {
				if (stack.empty())
					return false;  // Found a closing brace without a corresponding open brace
				else
					stack.pop();  // Pop corresponding open brace from the stack
			}
		}
		
	}
    // Check if there are any unclosed braces left on the stack
    return stack.empty();
}

//Clean input from comments and put input in tokens
void	populateTokens(std::stringstream &bufferstream, std::deque<std::string>	&tokens) {
	std::string			line;
	char				limiter = '\0';

	tokens.push_back("");
	while (getline(bufferstream, line)) {
		
		std::size_t pos;
	
		line += '\n';
		while (!line.empty()) {
			//if looking for closing limiter copy everything until limiter or whole string
			if (limiter) {
				pos = line.find(limiter);
				tokens.back() += line.substr(0, pos);
				line = line.erase(0, pos);
				if (pos != std::string::npos){
					limiter = '\0';
					line = line.erase(0, 1);
				}
			}
			if (limiter == '\0') {
				// copy all tokens separated by space until first special character
				while (!strchr("#;\'\"", line[0])) {
					//create new token if space is found and current token is not empty
					if (isspace(line[0]) && tokens.back() != "")
						tokens.push_back("");
					//write nonspace char to token
					else if (line[0] && !isspace(line[0]))
						tokens.back() += line[0];
					line = line.erase(0, 1);
				}

				// erase comments after '#' and ';'
				if ('#' == line[0])
					line = line.erase();
				else if (line[0] == ';') {
					tokens.push_back(";");
					tokens.push_back("");
					line = line.erase();
				}
				//if string is limited within the line, add to line
				else if (!line.empty()){
					limiter = line[0];
					//erase limiter
					line = line.erase(0, 1);
					//look for closing limiter
					pos = line.find(limiter);
					// save 
					tokens.push_back(line.substr(0 , pos));
					if (pos == std::string::npos)
						line = line.erase();
					else {
						line = line.erase(0, ++pos);
						limiter = '\0';
					}
				}
			}
		}
	}
	tokens.pop_back();
}

std::deque<std::string>	readFile2Buffer (std::string filename){
	std::stringstream		bufferstream;
	std::deque<std::string>	tokens;

	size_t	fileend = filename.rfind(".conf");
	if ( fileend == std::string::npos || filename[fileend + 5]){
		throw std::invalid_argument ("Error: Invalid config file");
	}

	//check that file existst && try to acccess file
	std::ifstream	input(filename.c_str());
	if (!input.is_open())
		throw std::invalid_argument ("Error: Cannot open config file");
	//read file to buffer stringstream & close it
	bufferstream << input.rdbuf();
	input.close();

	//Check that brakets and quotes are balanced
	// if (!isBalanced(braketbuffer)) {
	// 	std::cerr << COLOR_ERROR  << "Error: Brakets or quotes are not balances" << std::endl << COLOR_STANDARD;
	// 	exit (1);
	// }

	//Create Tokens from Input
	populateTokens(bufferstream, tokens);
	bufferstream.clear();

	for (tokeniterator it = tokens.begin(); it != tokens.end(); it++) {
		if (*it == "")
			tokens.erase(it);
	}

	return (tokens);		
}

void removeConfDuplicates(std::vector<ServerConfig> &config){
	if (config.size() == 1)
		return ;
	//FIND AND STORE DUPLICATED HOSTS
	std::map < std::string, std::vector<ServerConfig*> > more_duplicates;
	for (std::vector<ServerConfig>::iterator it = config.begin(); it < config.end(); it++){
		std::string hostaddr = (*it).getHost();
		if (more_duplicates.find(hostaddr) != more_duplicates.end())
			continue;
		more_duplicates[hostaddr].push_back(&(*it));
		for (std::vector<ServerConfig>::iterator it2 = it + 1; it2 < config.end(); it2++){
			if ((*it2).getHost() == hostaddr){
				more_duplicates[hostaddr].push_back(&(*it2));
			}
		}
	}

	//RETURN IF NO DUPLICATES FOUND
	if (more_duplicates.empty())
		return ;
	
	//ITERATE THROUGH HOST DUPLICATES TO IDENTIFY AND REMOVE DUPLICATED PORTS. 
	//REMOVE SERVER CONFS WITH NO PORT LEFT
	for (std::map < std::string, std::vector<ServerConfig*> >::iterator it = more_duplicates.begin();
																		it != more_duplicates.end(); it++){
		std::set< size_t > ports2keep = it->second.front()->getListenPorts();
		for (std::vector<ServerConfig*>::iterator it2 = it->second.begin() + 1; it2 != it->second.end(); it2++){
			for (std::set< size_t >::iterator it_ports = ports2keep.begin(); it_ports != ports2keep.end(); it_ports++){
				std::cerr << COLOR_WARNING << "Warning: Duplicate of " << (*it2)->getHost() << ":" << *it_ports << " found. Will use first." << COLOR_STANDARD << std::endl;
				(*it2)->deletePort(*it_ports);
			}
		}
	}
	
	//REMOVE CONFS THAT HAVE NO PORT LEFT
	for (std::vector<ServerConfig>::iterator it = config.begin(); it < config.end(); it++){
			if ((it->getListenPorts()).empty()){
				config.erase(it);
				std::cerr << COLOR_WARNING << "Warning: No valid Port left. Deleting server." << COLOR_STANDARD << std::endl;
			}
	}
}

// Gets a dequeue of tokens, looks for server keyword and {} to identify serverblock and hand it to ServerConfig Class
std::vector<ServerConfig>	parseConfig (std::deque<std::string> tokens){
	std::vector<ServerConfig>		returnconfig;
	tokeniterator blockstart = tokens.begin();
	tokeniterator blockend;

	while (blockstart < tokens.end()) {
		
		//SKIP EMPTY TOKENS
		while (*blockstart == "")
			blockstart++;
		// HANDLE SERVER TOKENS
		if (*blockstart == "server")
		{	
			++blockstart;
			while (*blockstart == "")
				tokens.erase(blockstart);
			//CHECK FOR OPENING BRAKET AND FIND CLOSING TO PARSE BLOCK
			blockend = getClosingBraket(blockstart, tokens.end());
			if (*blockstart == "{" && (blockend != tokens.end())) {
				tokens.erase(tokens.begin(), ++blockstart);
				ServerConfig new_server(blockstart, blockend - 1);
				returnconfig.push_back(new_server);
				blockstart = blockend + 1;
			}
			else
				throw std::invalid_argument("missing closing } to:\n" + *(--blockstart) + " " + *(++blockstart) + " " + *(++blockstart) + " " + *(++blockstart));
		}
		else
			throw std::invalid_argument("not a server block: " + *blockstart);
	}
	removeConfDuplicates(returnconfig);
	return (returnconfig);
}

bool directoryExists (std::string dir_name) {
	struct stat info;
	if (stat(dir_name.c_str(), &info) != 0)
		return false;
	return (info.st_mode & S_IFDIR) != 0;
}

bool fileExists (std::string file_name) {
	std::ifstream file(file_name.c_str());
	return file.good();
}

void fillUnsetDirectives(std::vector<ServerConfig> &config) {
	if (config.empty())
		throw std::invalid_argument("Error: Server configuration lost while parsing.") ;
	std::deque<std::string> tokens = readFile2Buffer("config_files/default.conf");
	std::vector<ServerConfig> configs = parseConfig(tokens);
	ServerConfig defaultConf = configs.front();
	for (std::vector<ServerConfig>::iterator it = config.begin(); it < config.end(); it++){
		(*it).fillDirectives(defaultConf);
	}
}