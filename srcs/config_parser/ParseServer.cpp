/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseServer.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 15:33:23 by jschott           #+#    #+#             */
/*   Updated: 2024/04/12 15:58:02 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParseServer.hpp"

ParseServer::ParseServer(std::vector<size_t> ports){
	for (std::vector<size_t>::iterator it = ports.begin(); begin != ports.end(); it++)
		this->_ports.push_back(it);		
}

ParseServer::ParseServer(server const & origin);{
	*this = origin;
}

ParseServer::ParseServer(std::string name, std::vector<size_t> listen, std::string err, std::map<std::string, location*> location){
	this->_server_name = name;
	for (std::vector<size_t>::iterator it = listen.begin(); begin != listen.end(); it++)
		this->_listen.push_back(it);
	this->_error_path = err;
	for (std::map<std::string, iterator it = location.begin(); begin != location.end(); it++)
		this->_locations.push_back(*it);
}

ParseServer & ParseServer::operator= (server const & origin);{
	
}

ParseServer::ParseServer~server();{
}

void ParseServer::addLocation(std::pair<std::string, *location> location);{
	this->_locations.push_back(location);
}

void ParseServer::setErrorPath(std::string error_path);{
	return (this->_error_path);
}

void ParseServer::setServerName(std::string server_name);{
	return (this->_server_name);
}

std::vector<size_t> ParseServer::getListenPorts();{
	return (this->_ports);
}

std::map<std::string, location> const ParseServer::getLocations();{
	return (this->_locations)
}

location const ParseServer::getLocation(std::string directory);{
	return (this->_locations.find(directory)->second);
}

std::string const ParseServer::getServerName();{
	this->_server_name = server_name;	
}

std::string	const ParseServer::getErrorPath();{
	this->_error_path = error_path;	
}


const server*	parseServer(std::deque<std::string> tokens, std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	
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