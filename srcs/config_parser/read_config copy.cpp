/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_config copy.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/05 10:20:40 by jschott           #+#    #+#             */
/*   Updated: 2024/04/08 18:09:57 by jschott          ###   ########.fr       */
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
	std::stringstream	buffing;
	std::vector<std::string>	tokens;
	
	while (getline(bufferstream, line)) {
		
		std::size_t pos;
		std::size_t end;
		char		limiter = '\0';
	
		while (!line.empty()) {
			//if looking for closing limiter copy everything until limiter or whole string
			if (limiter) {
				pos = line.find(limiter);
				tokens.front() += line.substr(0, pos);
				line.erase(0, pos);
				if (pos = std::string::npos)
					limiter = '\0';
			}
			else {
				// copy all tokens separated by space until first special character
				for (char i = 0; !strchr("#\'\"", line[i]); i++) {
					if (isspace(line[i]) && tokens.front() != "")
						tokens.push_back("");
					else if (!isspace(line[i]))
						tokens.front() += line[i];
					line.erase(i, 1);
				}

				// erase comments
				if (line[0] == '#')
					line.erase(0);
			}
			
			else if (line[0]){
				limiter = line[0];
				line.erase(0, 1);
				pos = line.find(limiter);
				tokens.push_back(line.substr(0, pos));
				if (pos == std::string::npos)
					limiter = '\0';
			}

			//if string is limited within the line, add to line
			else if (line[0] == '\'' && 
				(pos = line.find('\'')) != std::string::npos) {
				tokens.push_back(line.substr(0 , pos));
				line.erase(0, pos);
			}

			else if (line[0] == '\'' && 
				(pos = line.find('\'')) == std::string::npos) {
				tokens.push_back(line);
				limiter = '\'';
			}
						
				//if string is limited within the line, add to line
				end = line.find('\'', pos);
				tokens.push_back(line.substr(pos, end));
				if (end == std::string::npos)
					tokens.push_back(line.[end]);
		}
	}

	//reset buffstream
	bufferstream.clear();
	bufferstream.str(buffer);

	// std::cout << "POST CLEANING CONTENT:" << std::endl << buffing.str() << std::endl << std::endl;
	

	
	
	//check server keyword
	std::vector<std::string>	servers;
	std::getline(buffing, buffer);
		// std::cout << buffer;
	while (std::getline(buffing, line)){
		// std::cout << "hello";
		if (!line.compare(0,7,"server{")){
			servers.push_back(buffer);
			std::cout << "SERVER BLOCK:" << std::endl << buffer << std::endl << std::endl;
			buffer = line + '\n';
			line.clear();
		}
		else
			buffer += line + "\n";
	}
	servers.push_back(buffer);
	// std::cout << buffer;

	//hand over server block to server parser
}

int main (int argc, char** argv){

	if (argc < 2)
		return (1);

	readFile2Buffer(argv[1]);

	return (0);
}