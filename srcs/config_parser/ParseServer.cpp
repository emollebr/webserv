/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseServer.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 15:33:23 by jschott           #+#    #+#             */
/*   Updated: 2024/04/15 16:49:13 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParseServer.hpp"
#include "ParseLocation.hpp"

ParseServer::ParseServer(std::vector<size_t> ports){
	for (std::vector<size_t>::iterator it = ports.begin(); it < ports.end(); it++)
		this->_listen.push_back(*it);		
}

ParseServer::ParseServer(ParseServer const & origin) {
	*this = origin;
}

ParseServer::ParseServer(std::string name, std::vector<size_t> listen, std::string err, std::map<std::string, ParseLocation*> location){
	_server_name = name;
	_listen = listen;
	_error_path = err;
	_locations = location;
}

ParseServer & ParseServer::operator= (ParseServer const & origin) {
	if (this == &origin)
		return *this;
	_listen = origin._listen;
	_locations = origin._locations;
	_host = origin._host;
	_server_name = origin._server_name;
	_error_path = origin._error_path;
	return *this;
}

ParseServer::~ParseServer() {
	for (std::map<std::string, ParseLocation*>::iterator it = _locations.begin();
				it != _locations.end(); it++)
		delete ((*it).second);
}

void ParseServer::addLocation(std::pair<std::string, ParseLocation*> location) {
	_locations.insert(location);
}

void ParseServer::setErrorPath(std::string error_path) {
	_error_path = error_path;
}

void ParseServer::setServerName(std::string server_name) {
	_server_name = server_name;
}

std::vector<size_t> ParseServer::getListenPorts() {
	if (_listen.empty())
		return std::vector<size_t>();
	std::vector<std::size_t> ports;
	ports = _listen;
	return (ports);
}

std::map<std::string, ParseLocation*> const ParseServer::getLocations() {
	if (_listen.empty())
		return std::map<std::string, ParseLocation*>();
	std::map<std::string, ParseLocation*>	locations;
	locations = _locations;
	return (locations);
}

ParseLocation const ParseServer::getLocation(std::string directory) {
	if	(_locations.find(directory) != _locations.end())
		return (*(*_locations.find(directory)).second);
	return (*(*_locations.end()).second);
}

std::string const ParseServer::getServerName() {
	return _server_name;	
}

std::string	const ParseServer::getErrorPath() {
	return _error_path;	
}


const ParseServer*	parseServer(std::deque<std::string> tokens, std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	
	std::cout << "Parsing Server from " << *begin << " to " << *end << std::endl;	

	//TBD THIS IS WITHIN CLASSES
	/* std::set<std::string>	server_directives = {"listen", "location", "host",
												 "host", "error"};
	std::set<std::string>	location_directives = {"root", "index", "methods_allowed",
												"redirect", "CGI", "max_body_size",
												"default_file", "upload_location", 
												"cgi_extension", "allow_get", "allow_post"}; */
	
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
				ParseLocation(begin + 1, statementend - 1);
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
	return NULL;
}