/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_config copy.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/05 10:20:40 by jschott           #+#    #+#             */
/*   Updated: 2024/04/09 18:20:37 by jschott          ###   ########.fr       */
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

//NEEDS TO GO IN HEADER FILE
enum class TokenType {
    Server,
    Listen,
    Host,
    Location,
    Root,
	Variable
};

//Check if all brakets and quotes are closed correctly
bool isBalanced(std::stringstream& ss) {
    std::stack<char> stack;
    char c;
	std::string open = "{\'\"";
	std::string close = "}\'\"";
	
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
				// std::cout << line << std::endl;
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

				// erase comments
				if (strchr("#;", line[0]))
					line = line.erase();

				//if string is limited within the line, add to line
				else {
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
	std::stringstream braketbuffer;
	braketbuffer << input.rdbuf();
	input.close();

	//Check that brakets and quotes are balanced
	if (!isBalanced(braketbuffer)) {
		std::cerr << "Error: Brakets or quotes are not balances" << std::endl;
		exit (1);
	}

	//Create Tokens from Input
	std::deque<std::string>	tokens;
	populateTokens(bufferstream, tokens);
	bufferstream.clear();

	//Print Tokens [HELPER FUNCTION]
	for (std::deque<std::string>::iterator it = tokens.begin(); it != tokens.end(); it++)
		std::cout << *it << std::endl;

	//CHECK FIRST TOKEN
	while (!tokens.empty()) {
		
		std::string pos;
		//IF SERVER CHECK NEXT TOKEN
			pos = tokens.front();
			if (pos == TokenType::Server)
				std::c
			// IF ISDIRECTIVE CHECK FOR ;
				//IF FOUND CREATE TUPLE
			
			// IF IS BLOCK PARSE BLOCK
				// FIND OPENING BRAKET
					// CHECK NEXT TOKEN
					// PARSE DIRECTIVES base on ;
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