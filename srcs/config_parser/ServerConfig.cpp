/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 15:33:23 by jschott           #+#    #+#             */
/*   Updated: 2024/04/22 15:17:12 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
// #include "LocationConfig.hpp"


void ServerConfig::init(){
	std::string	directives[] = {"listen", "location", "host",
									"server_name", "error_path"};
	typedef void (ServerConfig::*ServerConfigFunction)(tokeniterator begin, tokeniterator end);							
	ServerConfigFunction functions[] = {&ServerConfig::validatePort, &ServerConfig::validateLocation, &ServerConfig::validateHost, 
										&ServerConfig::validateServerName, &ServerConfig::validateErrorPath};
	int size = sizeof(directives) / sizeof(directives[0]);
	for (int i = 0; i < size; i++){
		_directives_set[directives[i]] = false;
		_directives_validation_funcs[directives[i]] = functions[i];
	}
}

ServerConfig::ServerConfig(){
	init();
}

ServerConfig::ServerConfig(std::vector<size_t> ports){
	for (std::vector<size_t>::iterator it = ports.begin(); it < ports.end(); it++)
		this->_ports.push_back(*it);		
}

ServerConfig::ServerConfig(ServerConfig const & origin) {
	*this = origin;
}

ServerConfig::ServerConfig(std::vector<std::string> names, std::vector<size_t> listen, std::string err, std::map<std::string, LocationConfig> location){
	_server_names = names;
	_ports = listen;
	_error_path = err;
	_locations = location;
}

ServerConfig::ServerConfig(std::deque<std::string> tokens, tokeniterator begin, tokeniterator end){
	init();
	tokeniterator statementend;	
	
	while (begin < end) {
		while (*begin == "")
			begin++;
		if (*begin == "location"){
			begin++;
			while (*begin == "")
				begin++;
			std::string	location_name = *begin++;
			while (*begin == "")
				begin++;
			//CHECK FOR OPENING BRAKET AND FIND CLOSING TO PARSE BLOCK
			if (begin != end && *begin == "{" &&
					((statementend = getClosingBraket(tokens, begin, end)) <= end)){
				_locations[location_name] = LocationConfig(begin + 1, statementend - 1);;
				begin = statementend + 1;
			}
			else
				return ;
			
		}
		// IF IS DIRECTIVE CHECK FOR ';', IF FOUND CREATE DIRECTIVE
		else if ((statementend = std::find(begin, end, ";")) <= end ) {
			parseServerDirective(begin, statementend - 1);
			begin = statementend + 1;
		}

		else 
			return ;		
	}
	std::cout << *this << std::endl;
}

ServerConfig & ServerConfig::operator= (ServerConfig const & origin) {
	if (this == &origin)
		return *this;
	_ports = origin._ports;
	_locations = origin._locations;
	_host = origin._host;
	_server_names = origin._server_names;
	_error_path = origin._error_path;
	return *this;
}

ServerConfig::~ServerConfig() {
}

/* SETTER */
void ServerConfig::addLocation(std::string location, LocationConfig config) {
	if (_locations.find(location) == _locations.end())
		// return ;
		_locations[location] = config;
	return ;
}

void ServerConfig::setErrorPath(std::string error_path) {
	if ((_directives_set.find("error_path") != _directives_set.end() )
			&& (*_directives_set.find("error_path")).second)
		_error_path = error_path;
	// else
		// throw exception
}

void ServerConfig::setServerName(std::vector<std::string> server_names) {
	if ((_directives_set.find("server_name") != _directives_set.end() )
			&& (*_directives_set.find("server_name")).second)
	_server_names = server_names;
	// else
		// throw exception
}

/* GETTER */
std::vector<size_t> ServerConfig::getListenPorts() const{
	if (_ports.empty())
		return std::vector<size_t>();
	std::vector<std::size_t> ports;
	ports = _ports;
	return (ports);
}

std::string ServerConfig::getHost() const{
	return _host;
}

std::map<std::string, LocationConfig> const ServerConfig::getLocations() const{
	if (_ports.empty())
		return std::map<std::string, LocationConfig>();
	std::map<std::string, LocationConfig>	locations;
	locations = _locations;
	return (locations);
}

LocationConfig const ServerConfig::getLocation(std::string directory) const {
	if	(_locations.find(directory) != _locations.end())
		return ((*_locations.find(directory)).second);
	return ((*_locations.end()).second);
}

std::vector<std::string> const ServerConfig::getServerNames() const{
	if ((*_directives_set.find("server_name")).second)
		return _server_names;
	std::vector<std::string> const test;
	return test;
	// throw std::expection;
}

std::string	const ServerConfig::getErrorPath() const{
	return _error_path;	
}

void	ServerConfig::parseServerDirective(tokeniterator begin, 
											tokeniterator end){
/* 	if (_directives_set.find(*begin) == _directives_set.end())
		throw InvalidDirectiveException(); // no parameters
	if ((*_directives_set.find(*begin)).second)
		throw InvalidDirectiveException(); // no known directive found */
	std::map<std::string, void (ServerConfig::*)
		(tokeniterator, tokeniterator)>
		::iterator function = _directives_validation_funcs.find(*begin);
	if (function != _directives_validation_funcs.end()) {
		try {
			(this->*(function->second))(begin + 1, end);
			(*_directives_set.find(*begin)).second = true;
		}
		catch (const InvalidDirectiveException& e){
			std::cerr << e.what() << std::endl;			
		}
	}
/* 	else
		throw InvalidDirectiveException(); // no validation function found */
}


void	ServerConfig::validatePort(tokeniterator begin, tokeniterator end){
	while (begin <= end)
		_ports.push_back((size_t) std::atoi((*(begin++)).c_str()));
	//DO VALIDATION
}

void	ServerConfig::validateLocation(tokeniterator begin, tokeniterator end){
	if (begin == end)
		_locations[*begin] = LocationConfig(begin, end);
	//DO VALIDATION
}

void	ServerConfig::validateHost(tokeniterator begin, tokeniterator end){
	if (begin == end)
		_host = *(begin);
	//DO VALIDATION
}

void	ServerConfig::validateServerName(tokeniterator begin, tokeniterator end){
	//DO VALIDATION
	while (begin <= end)
		_server_names.push_back(*begin++);
}

void	ServerConfig::validateErrorPath(tokeniterator begin, tokeniterator end){
	if (begin == end)
		_error_path = *(begin);
}

std::ostream& operator<<(std::ostream& os, const ServerConfig& serverconf) {
	os << "server	{" << std::endl;
	
	os << "	host		" << serverconf.getHost() << ";" << std::endl;
	
	os << "	ports		" ;
	std::vector<size_t> ports = serverconf.getListenPorts();
	for (std::vector<size_t>::iterator it = ports.begin(); it < ports.end(); it++)
		os << *it << " ";
	os << ";" << std::endl;

	try {
		os << "	error_path	" << serverconf.getErrorPath() << ";" << std::endl;
	
		os << "	server_name	";
		std::vector<std::string> server_names = serverconf.getServerNames();
		for (std::vector<std::string>::iterator it = server_names.begin(); it < server_names.end(); it++)
			os << *it << " ";
		os << ";" << std::endl;

		std::map<std::string, LocationConfig> locations = serverconf.getLocations();
		for (std::map<std::string, LocationConfig>::iterator it = locations.begin(); it != locations.end(); it++)
			os << "	location " << (*it).first << " {" << std::endl 
				<< ((*it).second) << "" << std::endl;
	}
	
	catch(const std::exception& e)	{
		// std::cerr << e.what() << '\n';
	}
	
	os << "}" << std::endl << std::endl;
	return os;
	
}