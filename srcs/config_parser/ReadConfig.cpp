/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_config.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/05 10:20:40 by jschott           #+#    #+#             */
/*   Updated: 2024/04/12 15:31:39 by jschott          ###   ########.fr       */
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
#include <tuple>

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
std::deque<std::string>::iterator getClosingBraket (std::deque<std::string>& queue, std::deque<std::string>::iterator start) {
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

	// std::cout << "finding closing limiter " << limiter_close << " to opening limiter " << limiter_open << std::endl;
	
	//iterate through dequeue and return closing limiter if found
	std::stack<std::string> stack;
	for (std::deque<std::string>::iterator pos = start + 1; pos < queue.end(); pos++) {
		if (*pos == limiter_close) {
			if (limiter_open == limiter_close || stack.empty())
				return pos;
			else if (!stack.empty())
				stack.pop();
		}
		else if (*pos == limiter_open)
			stack.push(*pos);
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
	std::cout << "		Parsing Directive from " << *begin << " to " << *end << std::endl;
	
}

void	parseLocation(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end) {
	std::cout << TEXT_BOLD << "	Parsing Location from " << *begin << " to " << *end << std::endl;
	std::deque<std::string>::iterator directiveend = std::find(begin, end, ";") ;
	//IF FOUND ; CREATE TUPLE
	while (begin < end){
		directiveend = std::find(begin, end, ";") ;
		if (directiveend <= end) {
			parseDirective(begin, directiveend - 1);
			begin = directiveend + 1;
		}
		else {
			std::cerr << COLOR_ERROR << "Error: Cannot parse directive" << std::endl << COLOR_STANDARD;
			return ;
		}
	}
	std::cout << TEXT_NOFORMAT;
}

void	parseServer(std::deque<std::string> tokens, std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	
	std::cout << "Parsing Server from " << *begin << " to " << *end << std::endl;	

	//TBD THIS IS WITHIN CLASSES
	std::set<std::string>	server_directives = {"listen", "location", "host",
												 "host", "error"};
	std::set<std::string>	location_directives = {"root", "index", "methods_allowed",
												"redirect", "CGI", "max_body_size",
												"default_file", "upload_location", 
												"cgi_extension", "allow_get", "allow_post"};
	
	std::deque<std::string>::iterator statementend;

	while (begin < end) {
		if (*begin == "location"){
			begin++;
			while (*begin == "")
				begin++;
			std::pair<std::string, std::string> location_block;
			location_block.first = *begin++;
			while (*begin == "")
				begin++;
			//CHECK FOR OPENING BRAKET AND FIND CLOSING TO PARSE BLOCK
			if (begin != end && *begin == "{" &&
					((statementend = getClosingBraket(tokens, begin)) <= tokens.end())){
				parseLocation(begin + 1, statementend - 1);
				begin = statementend + 1;
			}
		}
		// IF IS DIRECTIVE CHECK FOR ';', IF FOUND CREATE TUPLE
		else if ((statementend = std::find(begin, end, ";")) <= end ) {
			parseDirective(begin, statementend - 1);
			begin = std::find(begin, end, ";") + 1;
		}

		else
			std::cerr << COLOR_ERROR  << "Error: Cannot parse config." << std::endl << COLOR_STANDARD;
		
	}
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
						line = line.erase(0, pos);
					else {
						line = line.erase();
						limiter = '\'';
					}
				}
			}
		}
	}
	tokens.pop_back();
}

void	readFile2Buffer (std::string filename){
	std::stringstream	bufferstream;
	std::ifstream		input(filename);
	

	//check that file existst && try to acccess file
	if (!input.is_open()){
		std::cerr << COLOR_ERROR  << "Error: Couldn't open file" << std::endl << COLOR_STANDARD;
		exit (1);	
	}
	//read file to buffer stringstream & close it
	bufferstream << input.rdbuf();
	input.close();

	//Check that brakets and quotes are balanced
	// if (!isBalanced(braketbuffer)) {
	// 	std::cerr << COLOR_ERROR  << "Error: Brakets or quotes are not balances" << std::endl << COLOR_STANDARD;
	// 	exit (1);
	// }

	//Create Tokens from Input
	std::deque<std::string>	tokens;
	populateTokens(bufferstream, tokens);
	bufferstream.clear();

	// printTokens(tokens);

	//CHECK FIRST TOKEN
	while (!tokens.empty()) {
		
		
		//SKIP EMPTY TOKENS
		while (tokens.front() == "")
			tokens.pop_front();
		std::cout << tokens.front() << std::endl;
		// HANDLE SERVER TOKENS
		if (!tokens.empty() && tokens.front() != "server") {
			std::cerr << COLOR_ERROR  << "Error: Cannot parse line:" << tokens.front() << std::endl << COLOR_STANDARD;
			break ;
		}
		else if (!tokens.empty()) {	
			tokens.pop_front();
			while (tokens.front() == "")
				tokens.pop_front();
			std::deque<std::string>::iterator blockstart = tokens.begin();
			std::deque<std::string>::iterator blockend;
		// std::cout << tokens.front() << std::endl;
			//CHECK FOR OPENING BRAKET AND FIND CLOSING TO PARSE BLOCK
			if (*blockstart == "{" &&
					((blockend = getClosingBraket(tokens, blockstart)) != tokens.end())) {
				tokens.erase(tokens.begin(), ++blockstart);
				parseServer(tokens, blockstart, blockend - 1);
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