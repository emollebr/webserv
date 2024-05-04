/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 15:33:23 by jschott           #+#    #+#             */
/*   Updated: 2024/05/03 17:41:37 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
// #include "LocationConfig.hpp"


void ServerConfig::init(){
	std::string	directives[] = {"port", "location", "host",
									"server_name", "error_page", "listen"};
	typedef void (ServerConfig::*ServerConfigFunction)(tokeniterator begin, tokeniterator end);							
	ServerConfigFunction functions[] = {&ServerConfig::validatePort, &ServerConfig::validateLocation, &ServerConfig::validateHost, 
										&ServerConfig::validateServerName, &ServerConfig::validateErrorPath, &ServerConfig::validateHostPort};
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
		this->_ports.insert(*it);		
}

ServerConfig::ServerConfig(ServerConfig const & origin) {
	*this = origin;
}

ServerConfig::ServerConfig(tokeniterator begin, tokeniterator end){
	init();
	tokeniterator statementend;
	std::string	location_name;
	
	while (begin < end) {
		while (*begin == "")
			begin++;
		if (*begin == "location"){
			begin++;
			while (*begin == "")
				begin++;
			location_name = *begin++;
			while (*begin == "")
				begin++;
			//CHECK FOR OPENING BRAKET AND FIND CLOSING TO PARSE BLOCK
			if (begin != end && *begin == "{" &&
					((statementend = getClosingBraket(begin, end)) <= end)){
				try	{
					if (_locations.find(location_name) != _locations.end())
						std::cerr << COLOR_WARNING << "Warning: duplicated location block " << location_name << ". Using last." << COLOR_STANDARD << std::endl;
					_locations[location_name] = LocationConfig(begin + 1, statementend - 1);
				}
				catch(const std::exception& e) {
					throw std::invalid_argument("location " + location_name + ": " + e.what());
				}
			}
			else
				throw std::invalid_argument("location block " + *begin + " missing closing '}'");
		}
	// IF IS DIRECTIVE CHECK FOR ';', IF FOUND CREATE DIRECTIVE
	else if ((statementend = std::find(begin, end + 1, ";")) <= end ) {
		try {
			parseServerDirective(begin, statementend - 1);
		}
		catch(const std::exception& e) {
			throw std::invalid_argument(e.what());
		}
		
	}
	else {
		throw std::invalid_argument("server directive: " + *begin + " missing closing ';'");
	}
	begin = statementend + 1;		
	}	
}

ServerConfig & ServerConfig::operator= (ServerConfig const & origin) {
	if (this == &origin)
		return *this;
	_ports = origin._ports;
	_locations = origin._locations;
	_host = origin._host;
	_server_names = origin._server_names;
	_error_pages = origin._error_pages;
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

void ServerConfig::setServerName(std::vector<std::string> server_names) {
	if ((_directives_set.find("server_name") != _directives_set.end() )
			&& (*_directives_set.find("server_name")).second)
	_server_names = server_names;
	// else
		// throw exception
}

/* GETTER */
std::set<std::pair <std::string, size_t> > ServerConfig::getListen() const{
	
	// if (_host_ports_registry.empty()){
	// 	std::cout << "nothing to see here" << std::endl;
	// 	return std::set<std::pair <std::string, size_t> >();
	// }
	// _host_ports_registry.insert(test);
	std::set<std::pair <std::string, size_t> > listen;
	listen = _host_ports_registry;
	return listen;
}

std::set<size_t> ServerConfig::getListenPorts() const{
	if (_ports.empty())
		return std::set<size_t>();
	std::set<std::size_t> ports;
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

std::map<uint, std::string>	const ServerConfig::getErrorPages() const {
	if (_error_pages.empty())
		return std::map<uint, std::string>();
	std::map<uint, std::string>	error_pages;
	error_pages = _error_pages;
	return error_pages;
}

std::string	const ServerConfig::getErrorPath(int statusCode) const{
	if (_error_pages.find(statusCode) != _error_pages.end())
		return (*_error_pages.find(statusCode)).second;
	return NULL;
}

void	ServerConfig::parseServerDirective(tokeniterator begin, tokeniterator end){
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
		catch (const std::exception& e){
			throw std::invalid_argument("server directive " + *begin + ": " + e.what());
		}
	}
	else
		throw std::invalid_argument("invalid server directive: " + *begin);
}


void	ServerConfig::validatePort(tokeniterator begin, tokeniterator end){
	for (NULL; begin <= end; begin++) {
		char* error = NULL;
		unsigned long int port = strtoul((*begin).c_str(), &error, 0);
		if (strlen(error) > 0 || port > 65535)
			throw std::invalid_argument("invalid parameter: " + *begin);
		if (port > 49151)
			std::cerr << COLOR_WARNING << "Warning: Unusual status port: " << port << COLOR_STANDARD << std::endl;
		if (find(_ports.begin(), _ports.end(), port) == _ports.end())
			_ports.insert(port);
	}
}

void	ServerConfig::validateLocation(tokeniterator begin, tokeniterator end){
	if (begin == end)
		return ;
	else
		throw std::invalid_argument("missing parameter.");
	// if (begin == end) {
	try	{
			_locations[*begin] = LocationConfig(begin, end);
	}
	catch(const std::exception& e)	{
		std::cout << *begin << std::endl << std::endl;
		throw std::invalid_argument(e.what());
	}
}

bool	ServerConfig::isValidatePort(char* port_str){
	char* error = NULL;
	unsigned long int port = strtoul(port_str, &error, 0);
	if (strlen(error) > 0 || port > 65535)
		throw std::invalid_argument("invalid parameter: ");
	if (port > 49151)
		std::cerr << COLOR_WARNING << "Warning: Unusual status port: " << port << COLOR_STANDARD << std::endl;
	return true;
}

bool	ServerConfig::isValidateHost(char* host){
	if (!host)
		return true;
	char * IP = new char [strlen(host)];
	strcpy(IP, host);
	IP =  strtok(IP, ".");
	for ( int i = 0; IP != NULL && i < 4; i++){
		char* error = NULL;
		unsigned long int body = strtoul(IP, &error, 0);
		if (strlen(error) > 0 || body > 255)
			throw std::invalid_argument("invalid parameter: ");
		IP = strtok(NULL, ".");
	}
	return true;
}

void	ServerConfig::validateHostPort(tokeniterator begin, tokeniterator end){
	char *tkns = new char [(*begin).length() + 1];
	char* host = new char [strlen(tkns)];
	_host_ports_registry.insert(std::make_pair("test", 13));
	
	if (begin == end){
		
		strcpy(tkns, (*begin).c_str());
		host = strtok(tkns, ":");
		tkns = strtok(NULL, ":");
		if (isValidateHost(host) && isValidatePort(tkns)){
			size_t port = strtoul(tkns, NULL, 0);
			std::pair<std::string, size_t> test(host, port);
			// _host_ports_registry.insert(test);
			_host_ports_registry.insert(std::make_pair(host, port));
			// _host_ports_registry.insert(std::make_pair("test", 13));
			std::set<std::pair <std::string, size_t> > host_ports = _host_ports_registry;
			for (std::set<std::pair <std::string, size_t> >::iterator it = host_ports.begin(); it != host_ports.end(); it++) {
				std::cout << "\tlisten\t\t" << (*it).first << ":" << (*it).second << std::endl;
			}
		}
			
	}
}

void	ServerConfig::validateHost(tokeniterator begin, tokeniterator end){
	if (begin == end){
		char *tkns = new char [(*begin).length() + 1];
		strcpy(tkns, (*begin).c_str());
		tkns =  strtok(tkns, ".");
		while (tkns != NULL){
			char* error = NULL;
			unsigned long int body = strtoul(tkns, &error, 0);
			if (strlen(error) > 0 || body > 255)
				throw std::invalid_argument("invalid parameter: " + *begin);
			tkns = strtok(NULL, ".");
		}
		_host = *begin;
	}
	else
		throw std::invalid_argument("invalid number of parameters.");
}

void	ServerConfig::validateServerName(tokeniterator begin, tokeniterator end){
	//DO VALIDATION
	while (begin <= end)
		_server_names.push_back(*begin++);
}

void	ServerConfig::validateErrorPath(tokeniterator begin, tokeniterator end){
	
	if (begin >= end || *end == "")
		throw std::invalid_argument("invalid number of prameters.");
		
	std::string errorPage = *end;
	if (!fileExists(errorPage))
		throw std::invalid_argument("invalid error file: " + *end);
	--end;

	for (NULL; begin <= end; begin++){
		char * error = NULL;
		unsigned int statusCode = strtoul((*begin).c_str(), &error, 0);
		if (strlen(error) > 0 || statusCode < 100 || statusCode > 599)
			throw std::invalid_argument("invalid parameter: " + *begin);
		if (statusCode < 400)
			std::cerr << COLOR_WARNING << "Warning: unusual status code for error_pages: " << statusCode << COLOR_STANDARD << std::endl;
		if (_error_pages.find(statusCode) != _error_pages.end())
			std::cerr << COLOR_WARNING << "Warning: multiple error_pages for http status code: " << statusCode << ". Will use last." << COLOR_STANDARD << std::endl;
		_error_pages[statusCode] = errorPage;
		_directives_set["return"] = true;
	}
}

void	ServerConfig::deletePort(size_t port){
	std::cout << "erasing: " << port << std::endl;
	if (_ports.find(port) == _ports.end())
		return ;
	std::cout << "erasing: " << port << std::endl;
	_ports.erase(port);
	_ports.erase(8080);
}

std::ostream& operator<<(std::ostream& os, const ServerConfig& serverconf) {
	os << "server\t{" << std::endl;
	
	os << "\thost\t\t" << serverconf.getHost() << ";" << std::endl;
	
	os << "\tports\t\t" ;
	std::set<size_t> ports = serverconf.getListenPorts();
	for (std::set<size_t>::iterator it = ports.begin(); it != ports.end(); it++) {
		if (it != ports.begin())
			os << " ";
		os << *it;
	}
	os << ";" << std::endl;


/* 	std::set<std::pair <std::string, size_t> > host_ports = serverconf.getListen();
	for (std::set<std::pair <std::string, size_t> >::iterator it = host_ports.begin(); it != host_ports.end(); it++) {
		os << "\tlisten\t\t" << (*it).first << ":" << (*it).second << std::endl;
	} */


 	std::map<uint, std::string> error_pages = serverconf.getErrorPages();

	for (std::map<uint, std::string>::iterator it = error_pages.begin(); it != error_pages.end(); it++){
		
	}
		

	while (!error_pages.empty()){	
		std::cout << "\terror_page\t";
		std::string path = (*error_pages.begin()).second;
		std::stack<std::map<uint, std::string>::iterator> trash;
		for (std::map<uint, std::string>::iterator it = error_pages.begin(); it != error_pages.end(); it++) {
			if (path == (*it).second){
				std::cout << (*it).first << " ";
				trash.push(it);
				// error_pages.erase(it);
				// break ;
			}
		}
		while (!trash.empty()){
			error_pages.erase(trash.top());
			trash.pop();
		}
		std::cout << path << ";" << std::endl;
	}
		
	os << "\tserver_name\t";
	std::vector<std::string> server_names = serverconf.getServerNames();
	for (std::vector<std::string>::iterator it = server_names.begin(); it < server_names.end(); it++)
		os << *it << " ";
	os << ";" << std::endl;

	std::map<std::string, LocationConfig> locations = serverconf.getLocations();
	for (std::map<std::string, LocationConfig>::iterator it = locations.begin(); it != locations.end(); it++)
		os << "\tlocation\t" << (*it).first << " {" << std::endl 
			<< ((*it).second) << "" << std::endl;
	
	os << "}" << std::endl << std::endl;
	return os;
	
}