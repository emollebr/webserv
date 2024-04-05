/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_config.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/05 10:20:40 by jschott           #+#    #+#             */
/*   Updated: 2024/04/05 15:49:25 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
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

	//Clean input from spaces and comments
	std::string 		buffer = "";
	std::string			line;
	std::stringstream	buffing;
	while (std::getline(bufferstream, line)) {
		
		//delete spaces
		line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());

		//Delete comments
		std::size_t pos = line.find('#');
		if (pos != std::string::npos) {
				line.erase(pos);
		}
		
		buffing << line << std::endl;
		// buffer += line + "\n";
	}
	bufferstream.clear();
	bufferstream.str(buffer);

	// std::cout << "POST CLEANING CONTENT:" << std::endl \
				<< buffing.str() << std::endl << std::endl;
	

	//make sure that every curly braket will be closed
	if (!isBalanced(bufferstream)) {
		std::cerr << "Brakets are not balanced" << std::endl;
		exit (1);
	}
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
			buffer += line;
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