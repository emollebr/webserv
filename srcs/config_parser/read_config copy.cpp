/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_config copy.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/05 10:20:40 by jschott           #+#    #+#             */
/*   Updated: 2024/04/09 11:50:46 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <stack>
#include <vector>

bool isBalanced(std::stringstream& ss) {
    std::stack<char> stack;
    char c;

    while (ss.get(c)) {
        if (c == '{') {
            stack.push(c);  // Push open brace onto the stack
        } else if (c == '}') {
            if (stack.empty()) {
                return false;  // Found a closing brace without a corresponding open brace
            } else {
                stack.pop();  // Pop corresponding open brace from the stack
            }
        }
    }

    // Check if there are any unclosed braces left on the stack
    return stack.empty();
}

/* void* endofstring(std::stringstream& ss) {
	
	
	
} */

void	readFile2Buffer (std::string filename){
	std::stringstream	bufferstream;
	std::ifstream		input(filename);
	

	//check that file existst && try to acccess file
	if (!input.is_open()){
		std::cerr << "couldn't open file" << std::endl;
		exit (1);	
	}
	//read file to buffer stringstream & close it
	bufferstream << input.rdbuf();
	input.close();

	//Clean input from 

	//Clean input from spaces and comments
	std::string 		buffer = "";
	std::string			line;
	std::vector<std::string>	tokens;
	char				limiter = '\0';
	
	tokens.push_back("");
	while (getline(bufferstream, line)) {
		
		std::size_t pos;
	
		while (!line.empty()) {
			//if looking for closing limiter copy everything until limiter or whole string
			if (limiter) {
				pos = line.find(limiter);
				tokens.back() += line.substr(0, pos);
				line = line.erase(0, pos);
				if (pos == std::string::npos)
					limiter = '\0';
			}
			if (limiter == '\0') {
				// copy all tokens separated by space until first special character
				while (line[0] && !strchr("#\'\"", line[0])) {
					// if (!isspace(line[0]))
						// std::cout << line[0] << " ";
				
					//create new token if space is found and current token is not empty
					if (isspace(line[0]) && tokens.back() != "")
						tokens.push_back("");
					//write nonspace char to token
					else if (line[0] && !isspace(line[0]))
						tokens.back() += line[0];
					line = line.erase(0, 1);
					// std::cout << line << std::endl;
				}

				// erase comments
				if (line[0] == '#')
					line = line.erase();

				//if string is limited within the line, add to line
				else if (line[0] == '\'') {
					//erase limiter
					line = line.erase(0, 1);
					//look for closing limiter
					pos = line.find('\'');
					// save 
					// std::cout <<  line.substr(0 , pos) << std::endl;
					tokens.push_back(line.substr(0 , pos));
					if (pos == std::string::npos) {
						limiter = '\'';
					// std::cout << "limiter: " << limiter << std::endl;
						line = line.erase(0, pos);
					}
					else
						line = line.erase();
					// std::cout << "line: " << line << std::endl;
				}
			}
		}
	}

	for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); it++)
		std::cout << *it << std::endl;
	
	//reset buffstream
	// bufferstream.clear();
	// bufferstream.str(buffer);

	// std::cout << "POST CLEANING CONTENT:" << std::endl << buffing.str() << std::endl << std::endl;
	

	
	
	//check server keyword
	// std::vector<std::string>	servers;
	// std::getline(buffing, buffer);
	// 	// std::cout << buffer;
	// while (std::getline(buffing, line)){
	// 	// std::cout << "hello";
	// 	if (!line.compare(0,7,"server{")){
	// 		servers.push_back(buffer);
	// 		std::cout << "SERVER BLOCK:" << std::endl << buffer << std::endl << std::endl;
	// 		buffer = line + '\n';
	// 		line.clear();
	// 	}
	// 	else
	// 		buffer += line + "\n";
	// }
	// servers.push_back(buffer);
	// std::cout << buffer;

	//hand over server block to server parser
}

int main (int argc, char** argv){

	if (argc < 2)
		return (1);

	readFile2Buffer(argv[1]);

	return (0);
}