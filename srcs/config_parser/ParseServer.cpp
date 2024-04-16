/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseServer.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 15:33:23 by jschott           #+#    #+#             */
/*   Updated: 2024/04/16 18:01:20 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParseServer.hpp"
#include "ParseLocation.hpp"


ParseServer::ParseServer(){
	std::string	directives[] = {"listen", "location", "host",
									"server_name", "error_path"};
	typedef void (ParseServer::*ParseServerFunction)(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);							
	ParseServerFunction functions[] = {&ParseServer::validatePort, &ParseServer::validateLocation, &ParseServer::validateHost, 
										&ParseServer::validateServerName, &ParseServer::validateErrorPath};
	int size = sizeof(directives) / sizeof(directives[0]);
	for (int i = 0; i < size; i++){
		_directives_index[directives[i]] = false;
		_validation_index[directives[i]] = functions[i];
	}
}

ParseServer::ParseServer(std::vector<size_t> ports){
	for (std::vector<size_t>::iterator it = ports.begin(); it < ports.end(); it++)
		this->_ports.push_back(*it);		
}

ParseServer::ParseServer(ParseServer const & origin) {
	*this = origin;
}

ParseServer::ParseServer(std::vector<std::string> names, std::vector<size_t> listen, std::string err, std::map<std::string, ParseLocation*> location){
	_server_names = names;
	_ports = listen;
	_error_path = err;
	_locations = location;
}

ParseServer & ParseServer::operator= (ParseServer const & origin) {
	if (this == &origin)
		return *this;
	_ports = origin._ports;
	_locations = origin._locations;
	_host = origin._host;
	_server_names = origin._server_names;
	_error_path = origin._error_path;
	return *this;
}

ParseServer::~ParseServer() {
	for (std::map<std::string, ParseLocation*>::iterator it = _locations.begin();
				it != _locations.end(); it++)
		delete ((*it).second);
}

/* SETTER */
void ParseServer::addLocation(std::string location, ParseLocation* config) {
	if (_locations.find(location) == _locations.end())
		_locations[location] = config;
	return ;
}

void ParseServer::setErrorPath(std::string error_path) {
	if ((_directives_index.find("error_path") != _directives_index.end() )
			&& (*_directives_index.find("error_path")).second)
		_error_path = error_path;
	// else
		// throw exception
}

void ParseServer::setServerName(std::vector<std::string> server_names) {
	if ((_directives_index.find("server_name") != _directives_index.end() )
			&& (*_directives_index.find("server_name")).second)
	_server_names = server_names;
	// else
		// throw exception
}

/* GETTER */
std::vector<size_t> ParseServer::getListenPorts() {
	if (_ports.empty())
		return std::vector<size_t>();
	std::vector<std::size_t> ports;
	ports = _ports;
	return (ports);
}

std::map<std::string, ParseLocation*> const ParseServer::getLocations() {
	if (_ports.empty())
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

std::vector<std::string> const ParseServer::getServerName() {
	return _server_names;	
}

std::string	const ParseServer::getErrorPath() const{
	return _error_path;	
}

void	ParseServer::parseServerDirective(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (_directives_index.find(*begin) == _directives_index.end())
		return ; //THROW EXCEPTION
	std::map<std::string, void (ParseServer::*)(std::deque<std::string>::iterator, std::deque<std::string>::iterator)>::iterator function = _validation_index.find(*begin);
	if (function != _validation_index.end()) {
		(this->*(function->second))(begin, end);
	}
}


void	ParseServer::validatePort(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		return ;
	//DO VALIDATION
	size_t port = (size_t) std::atoi((*(++begin)).c_str());
	_ports.push_back(port);
}

void	ParseServer::validateLocation(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		return ;
	// size_t port = (size_t) std::atoi((*(++begin)).c_str());
	//DO VALIDATION
	ParseLocation *location = new ParseLocation(begin, end);
	++begin;
	_locations[*begin] = location;
}

void	ParseServer::validateHost(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		return ;
	//DO VALIDATION
	_host = *(++begin);
}

void	ParseServer::validateServerName(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		return ;
	//DO VALIDATION
	while (++begin < end)
		_server_names.push_back(*begin);
}

void	ParseServer::validateErrorPath(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	if (begin == end)
		return ;
	//DO VALIDATION
	_error_path = *(++begin);
}

const ParseServer*	parseServer(std::deque<std::string> tokens, std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end){
	
	std::cout << "Parsing Server from " << *begin << " to " << *end << std::endl;	

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

std::ostream& operator<<(std::ostream& os, const ParseServer& serverconf) {
	os << "+++START SERVER CONFIGURATION+++" << std::endl << std::endl;
	// os << "" << _ << std::endl;
	// os << "" << _ << std::endl;
	os << serverconf.getErrorPath() << std::endl;
	os << "+++ END SERVER CONFIGURATION +++" << std::endl << std::endl;
	return os;
	
}