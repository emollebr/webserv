/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_config.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/05 10:20:40 by jschott           #+#    #+#             */
/*   Updated: 2024/04/10 16:45:39 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#define TEXT_NOFORMAT	"\e[0m"
#define TEXT_BOLD		"\e[1m"
#define TEXT_UNDERLINE	"\e[4m"

#define COLOR_STANDARD	"\033[0m"
#define	COLOR_ERROR		"\033[31m"
#define COLOR_WARNING	"\033[33m"
#define COLOR_SUCCESS	"\033[92m"
#define COLOR_FOCUS		"\e[35m"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <stack>
#include <vector>
#include <set>

//NEEDS TO GO IN HEADER FILE
enum class TokenType {
    Server,
    Listen,
    Host,
    Location,
    Root,
	Variable
};

void	printTokens(std::deque<std::string> tokens){
	//Print Tokens [HELPER FUNCTION]
	for (std::deque<std::string>::iterator it = tokens.begin(); it != tokens.end(); it++)
		std::cout<< *it << std::endl;
}

// finds the closing limiter in a dequeue of tokens to a given opening limiter char
// returns iterator to the position, NULL if limiter is not closed within the dequeue
std::deque<std::string>::iterator getClosingBraket (std::deque<std::string> queue, std::deque<std::string>::iterator start) {
	std::string	open =	"{[(\'\"`";
	std::string	close =	"}])\'\"`";
	std::string	limiter_open = *start;
	std::string	limiter_close;
	std::deque<std::string>::iterator notfound = queue.end();

	//return NULL if limiter_open is unknows
	if (open.find(limiter_open) == open.length())
		return notfound;
	//set closing limiter
	limiter_close = close[open.find(limiter_open)];

	// std::cout << " looking for: " << limiter_close << std::endl;

	
	//iterate through dequeue and return closing limiter if found
	std::stack<std::string> stack;
	
	for (std::deque<std::string>::iterator pos = ++start; pos != queue.end(); pos++){

		if (*pos == limiter_close) {
			if (limiter_open == limiter_close || stack.empty())
				return pos;
			else if (!stack.empty()){
				stack.pop();
			}
		}
		else if (*pos == limiter_open){
			stack.push(*pos);
		}
	}
	return notfound;
}

//Check if all brakets and quotes are closed correctly
bool isBalanced(std::stringstream& ss) {
    std::stack<char> stack;
    char c;
	std::string open =	"{[(\'\"`";
	std::string close = "}])\'\"`";
	
	for (size_t i = 0; i < strlen(open.c_str()); i++) {
		// std::cout << "checking for: " << open[i] << " and " << close[i] << std::endl;
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

void	 parseDirective(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	std::cout << "Parsing from " << *begin << " to " << *end << std::endl;
	
	while (begin != end){
		std::cout  << *begin << std::endl;
		begin++;
	}

	if (begin == end)
		return ;
}

void	parseLocation(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end) {
	std::cout << "Parsing from " << *begin << " to " << *end << std::endl;
	while (begin != end){
		std::cout  << *begin << std::endl;
		begin++;
	}

}

void	parseBlock(std::deque<std::string> tokens, std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	
	// std::cout << "Parsing from " << *begin << " to " << *end << std::endl;
	// while (begin != end){
	// 	std::cout << *begin << std::endl;
	// 	begin++;
	// }
	

	//TBD THIS IS WITHIN CLASSES
	std::set<std::string>	server_directives = {"listen", "location", "host",
												 "host", "error"};
	std::set<std::string>	location_directives = {"root", "index", "methods_allowed",
												"redirect", "CGI", "max_body_size",
												"default_file", "upload_location", 
												"cgi_extension", "allow_get", "allow_post"};
	if (*begin == "location"){
		tokens.pop_front();
		while (tokens.front() == "")
			tokens.pop_front();
		std::deque<std::string>::iterator blockstart = tokens.begin();
		std::deque<std::string>::iterator blockend;
		//CHECK FOR OPENING BRAKET AND FIND CLOSING TO PARSE BLOCK
		if (*blockstart == "{" &&
				((blockend = getClosingBraket(tokens, blockstart)) != tokens.end())) {
			parseLocation(blockstart, blockend);
			tokens.erase(tokens.begin(), ++blockend);
		}
	}
	// IF IS DIRECTIVE CHECK FOR ;
	// if (*begin != server_directives.back()){
		std::deque<std::string>::iterator directiveend = begin;
		while (directiveend != end){
			//IF FOUND ; CREATE TUPLE
			if (*directiveend == ";") {
				parseDirective(begin + 1, directiveend);
				break ;
				}
			directiveend++;
		}
		if (directiveend == end) {
			std::cerr << "ERROR: Missing ; to end Directive" << std::endl;
		}
	// }
	
	// if (begin == end)
	// 	return ;
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
				// if (strchr("#", line[0]))
					line = line.erase();
				else if (line[0] == ';') {
					tokens.push_back(";");
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
						line = line.erase(0, pos);
					else {
						line = line.erase();
						limiter = '\'';
					}
				}
			}
		}
	}
}

void	readFile2Buffer (std::string filename){
	std::stringstream	bufferstream;
	std::ifstream		input(filename);
	

	//check that file existst && try to acccess file
	if (!input.is_open()){
		std::cerr << "Error: Couldn't open file" << std::endl;
		exit (1);	
	}
	//read file to buffer stringstream & close it
	bufferstream << input.rdbuf();
	input.close();

	//Check that brakets and quotes are balanced
	// if (!isBalanced(braketbuffer)) {
	// 	std::cerr << "Error: Brakets or quotes are not balances" << std::endl;
	// 	exit (1);
	// }

	//Create Tokens from Input
	std::deque<std::string>	tokens;
	populateTokens(bufferstream, tokens);
	bufferstream.clear();

	//CHECK FIRST TOKEN
	while (!tokens.empty()) {
		
		//SKIP EMPTY TOKENS
		while (tokens.front() == "")
			tokens.pop_front();
		// HANDLE SERVER TOKENS
		if (tokens.front() == "server") {
			tokens.pop_front();
			while (tokens.front() == "")
				tokens.pop_front();
			std::deque<std::string>::iterator blockstart = tokens.begin();
			std::deque<std::string>::iterator blockend;
			//CHECK FOR OPENING BRAKET AND FIND CLOSING TO PARSE BLOCK
			if (*blockstart == "{" &&
					((blockend = getClosingBraket(tokens, blockstart)) != tokens.end())) {
				parseBlock(tokens, blockstart, blockend);
				tokens.erase(tokens.begin(), ++blockend);
			}
		}
		// std::cout << "Next Up: " << tokens.front() << std::endl;
	}
	

	
	//HOW TO CLEAN A STRINGSTREAM
	//reset buffstream
	// bufferstream.clear();
	// bufferstream.str(buffer);

			
}

int main (int argc, char** argv){

	if (argc < 2)
		return (1);

	readFile2Buffer(argv[1]);

	return (0);
}

// DIREctives: listen, error, location, index, methods, root, php, CGI, exec, php, 