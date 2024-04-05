/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_config.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/05 10:20:40 by jschott           #+#    #+#             */
/*   Updated: 2024/04/05 14:08:35 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <stack>

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
	std::string 		buffer;
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

	//Delete spaces & tabs -> USE INPUTSTRINGSTREAM LIBRARY
	while (bufferstream.good()){
		char c;
		bufferstream.get(c);
		if (!std::isspace(c))
			buffer += c;
	}
	bufferstream.str("");
	bufferstream << buffer;

	//Clean input from spaces and comments
	while (std::getline(bufferstream, buffer)) {
		
		//Delete comments
		std::size_t pos = buffer.find('#');
		if (pos != std::string::npos) {
				buffer.erase(pos);
		}
		
		//delete spaces
		buffer.erase(std::remove_if(buffer.begin(), buffer.end(), isspace), buffer.end());
	}
	bufferstream.str("");
	bufferstream << buffer;
	
	if (!isBalanced(bufferstream)) {
		std::cerr << "Brakets are not balanced" << std::endl;
		exit (1);
	}
	//check server keyword
	//hand over server block to server parser
}