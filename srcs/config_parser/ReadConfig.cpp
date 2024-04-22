/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReadConfig.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/05 10:20:40 by jschott           #+#    #+#             */
/*   Updated: 2024/04/18 10:52:36 by jschott          ###   ########.fr       */
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
tokeniterator getClosingBraket (std::deque<std::string>& queue, tokeniterator start) {
	std::string	open =	"{[(\'\"`";
	std::string	close =	"}])\'\"`";
	std::string	limiter_open = *start;
	std::string	limiter_close;
	tokeniterator notfound = queue.end();

	//return NULL if limiter_open is unknows
	if (open.find(limiter_open) == open.length())
		return notfound;
	//set closing limiter
	limiter_close = close[open.find(limiter_open)];
	
	//iterate through dequeue and return closing limiter if found
	std::stack<std::string> stack;
	for (tokeniterator pos = start + 1; pos < queue.end(); pos++) {
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

void	 parseDirective(tokeniterator begin, tokeniterator end){
	std::cout << "		Parsing Directive from " << *begin << " to " << *end << std::endl;
	
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

ServerConfig	readFile2Buffer (std::string filename){
	std::stringstream	bufferstream;
	std::ifstream		input(filename.c_str());
	

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
	ServerConfig ret;
	// printTokens(tokens);

	//CHECK FIRST TOKEN
	while (!tokens.empty()) {
		

		//SKIP EMPTY TOKENS
		while (tokens.front() == "")
			tokens.pop_front();
		// HANDLE SERVER TOKENS
		if (!tokens.empty() && tokens.front() != "server") {
			std::cerr << COLOR_ERROR  << "Error: Cannot parse line:" << tokens.front() << std::endl << COLOR_STANDARD;
			break ;
		}
		else if (!tokens.empty()) {	
			tokens.pop_front();
			while (tokens.front() == "")
				tokens.pop_front();
			tokeniterator blockstart = tokens.begin();
			tokeniterator blockend;
			//CHECK FOR OPENING BRAKET AND FIND CLOSING TO PARSE BLOCK
			if (*blockstart == "{" &&
					((blockend = getClosingBraket(tokens, blockstart)) != tokens.end())) {
				tokens.erase(tokens.begin(), ++blockstart);
				ServerConfig test(tokens, blockstart, blockend - 1);
				ret = test;
				tokens.erase(tokens.begin(), ++blockend);
			}
		}
	}

	return ret;
}

// DIREctives: listen, error, location, index, methods, root, php, CGI, exec, php, 