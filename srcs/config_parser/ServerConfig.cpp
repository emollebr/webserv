/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 15:33:23 by jschott           #+#    #+#             */
/*   Updated: 2024/05/14 11:23:34 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
// #include "LocationConfig.hpp"


void ServerConfig::init(){
	std::string	directives[] = {"port", "host",
									"server_name", "error_page",
									"cgi_extension"};
	typedef void (ServerConfig::*ServerConfigFunction)(tokeniterator begin, tokeniterator end);							
	ServerConfigFunction functions[] = {&ServerConfig::validatePort, &ServerConfig::validateHost, 
										&ServerConfig::validateServerName, &ServerConfig::validateErrorPath,
										&ServerConfig::validateCGIExtension};
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
	_host = origin._host;
	_ports = origin._ports;
	_server_names = origin._server_names;
	_error_pages = origin._error_pages;
	_directives_set = origin._directives_set;
	_directives_validation_funcs = origin._directives_validation_funcs;
	_cgi_extension = origin._cgi_extension;
	_locations = origin._locations;
	return *this;
}

ServerConfig::~ServerConfig() {
}

/* SETTER */
void ServerConfig::addLocation(std::string location, LocationConfig config) {
	if (_locations.find(location) == _locations.end())
		_locations[location] = config;
	return ;
}

void ServerConfig::setServerName(std::set<std::string> server_names) {
	if ((_directives_set.find("server_name") != _directives_set.end() )
			&& (*_directives_set.find("server_name")).second)
	_server_names = server_names;
}

/* GETTER */
std::set<std::pair <std::string, size_t> > ServerConfig::getListen() const{
	
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

std::set<std::string> const ServerConfig::getServerNames() const{
	if ((*_directives_set.find("server_name")).second)
		return _server_names;
	std::set<std::string> const test;
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

std::map<std::string, std::string> ServerConfig::getCGIExtention() const{
	if (_error_pages.empty())
		return std::map<std::string, std::string>();
	std::map<std::string, std::string>	cgi_extension;
	cgi_extension = _cgi_extension;
	return cgi_extension;
}

void	ServerConfig::parseServerDirective(tokeniterator begin, tokeniterator end){

	std::map<std::string, void (ServerConfig::*)(tokeniterator, tokeniterator)> ::iterator
			function = _directives_validation_funcs.find(*begin);
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
	for (; begin <= end; begin++) {
		char* error = NULL;
		unsigned long int port = strtoul((*begin).c_str(), &error, 0);
		if (strlen(error) > 0 || port > 65535)
			throw std::invalid_argument("invalid parameter: " + *begin);
		if (port > 49151)
			std::cerr << COLOR_WARNING << "Warning: Unusual status port: " << port << COLOR_STANDARD << std::endl;
		_ports.insert(port);
	}
}

bool	ServerConfig::isValidPort(char* port_str){
	char* error = NULL;
	unsigned long int port = strtoul(port_str, &error, 0);
	if (strlen(error) > 0 || port > 65535)
		throw std::invalid_argument("invalid parameter: ");
	if (port > 49151)
		std::cerr << COLOR_WARNING << "Warning: Unusual status port: " << port << COLOR_STANDARD << std::endl;
	return true;
}

bool	ServerConfig::isValidHost(char* host){
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

void	ServerConfig::validateHost(tokeniterator begin, tokeniterator end){
	if (begin == end){
		int group = 0;
		char *tkns = new char [(*begin).length() + 1];
		strcpy(tkns, (*begin).c_str());
		tkns =  strtok(tkns, ".");
		while (tkns != NULL){
			if (group++ > 3)
				throw std::invalid_argument("invalid parameter: " + *begin);				
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
	
	if (begin > end)
		throw std::invalid_argument("invalid number of parameters.");
	while (begin <= end){
		_server_names.insert(*begin++);
	}
}

void	ServerConfig::validateErrorPath(tokeniterator begin, tokeniterator end){
	
	if (begin >= end)
		throw std::invalid_argument("invalid number of prameters.");
	
	std::string errorPage = *end;
	if (!fileExists(errorPage))
		throw std::invalid_argument("invalid error file: " + *end);
	--end;

	for (NULL; begin <= end; begin++){
		char * error = NULL;
		unsigned int statusCode = strtoul((*begin).c_str(), &error, 0);
		if (strlen(error) > 0 || statusCode < 100 || statusCode > 599)
			throw std::invalid_argument("invalid status code: " + *begin);
		if (statusCode < 400)
			std::cerr << COLOR_WARNING << "Warning: unusual status code for error_pages: " << statusCode << COLOR_STANDARD << std::endl;
		if (_error_pages.find(statusCode) != _error_pages.end())
			std::cerr << COLOR_WARNING << "Warning: multiple error_pages for http status code: " << statusCode << ". Will use last." << COLOR_STANDARD << std::endl;
		_error_pages[statusCode] = errorPage;
	}
	_directives_set["return"] = true;
}

void	ServerConfig::validateCGIExtension(tokeniterator begin, tokeniterator end){
	if (begin >= end)
		throw std::invalid_argument("invalid number of prameters.");
	
	std::string extension = *begin++;
	if (extension[0] != '.' || extension[1] == '\0')
		throw std::invalid_argument("invalid parameter: " + extension);
	for (int i = 1; extension[i] != '\0'; i++){
		if (!isalnum(extension[i]))
			throw std::invalid_argument("invalid parameter: " + extension);
	}

	if (begin > end)
		throw std::invalid_argument("invalid number of prameters.");

	std::string cgi_path = *begin;
	if (!directoryExists(cgi_path))
		throw std::invalid_argument("invalid cgi path: " + *end);

	if (_cgi_extension.find(extension) != _cgi_extension.end())
		std::cerr << COLOR_WARNING << "Warning: multiple cgi paths for file extension " << extension << ". Will use last." << COLOR_STANDARD << std::endl;
	_cgi_extension[extension] = cgi_path;
	_directives_set["cgi_extension"] = true;
}

void	ServerConfig::deletePort(size_t port){
	if (_ports.find(port) == _ports.end())
		return ;
	_ports.erase(port);
}

void	ServerConfig::fillDirectives(ServerConfig &reference){
	std::string	directives[] = {"port", "location", "host",
									"server_name", "error_page",
									"cgi_extension"};
	int size = sizeof(directives) / sizeof(directives[0]);
	for (int i = 0; i < size; i++){
		if (!(*_directives_set.find(directives[i])).second){
			switch (i) {
			case 0:
				_ports = reference.getListenPorts();
				break;
			case 1:
				if (_locations.empty())
					_locations = reference.getLocations();
				else {
					for (std::map<std::string, LocationConfig>::iterator it = _locations.begin();
									it != _locations.end(); it++)
						(*it).second.fillNestedLocation();
				}
				break;
			case 2:
				_host = reference.getHost();
				break;
			case 3:
				_server_names = reference.getServerNames();
				break;
			case 4:
				_error_pages = reference.getErrorPages();
				break;
			case 5:
				_cgi_extension = reference.getCGIExtention();
				break;
			default:
				continue ;
			}
		(*_directives_set.find(directives[i])).second = true;
		}
	}
}

std::ostream& operator<<(std::ostream& os, const ServerConfig& serverconf) {
	os << "server\t{" << std::endl;
	
	//PRINT host IP
	os << "\thost\t\t" << serverconf.getHost() << ";" << std::endl;
	
	//PRINT PORTS
	os << "\tports\t\t" ;
	std::set<size_t> ports = serverconf.getListenPorts();
	for (std::set<size_t>::iterator it = ports.begin(); it != ports.end(); it++) {
		if (it != ports.begin())
			os << " ";
		os << *it;
	}
	os << ";" << std::endl;

	//PRINT SERVER NAMES
	std::set<std::string> server_names = serverconf.getServerNames();
	if (!server_names.empty()){
		os << "\tserver_name\t";
		for (std::set<std::string>::iterator it = server_names.begin(); it != server_names.end(); it++)
			os << *it << " ";
		os << ";" << std::endl;
	}

	//PRINT ERROR PAGES
 	std::map<uint, std::string> error_pages = serverconf.getErrorPages();
	while (!error_pages.empty()){	
		os << "\terror_page\t";
		std::string path = (*error_pages.begin()).second;
		std::stack<std::map<uint, std::string>::iterator> trash;
		for (std::map<uint, std::string>::iterator it = error_pages.begin(); it != error_pages.end(); it++) {
			if (path == (*it).second){
				os << (*it).first << " ";
				trash.push(it);
			}
		}
		while (!trash.empty()){
			error_pages.erase(trash.top());
			trash.pop();
		}
		os << path << ";" << std::endl;
	}

	//PRINT CGI EXTENSION
	std::map<std::string, std::string> cgi_extension = serverconf.getCGIExtention();
	if (!cgi_extension.empty()){
		for (std::map<std::string, std::string>::iterator it = cgi_extension.begin(); it != cgi_extension.end(); it++)
		os << "\tcgi_extension\t" << (*it).first << " " << (*it).second << ";" << std::endl;
	}

	//PRINT LOCATION BLOCKS
	std::map<std::string, LocationConfig> locations = serverconf.getLocations();
	for (std::map<std::string, LocationConfig>::iterator it = locations.begin(); it != locations.end(); it++)
		os << "\tlocation\t" << (*it).first << " {" << std::endl 
			<< ((*it).second) << "" << std::endl;
	os << "}" << std::endl << std::endl;
	return os;
	
}