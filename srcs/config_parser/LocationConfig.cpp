/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 12:09:36 by jschott           #+#    #+#             */
/*   Updated: 2024/04/17 17:18:53 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

void LocationConfig::init(){
	_max_body_size = 4.2 * 1024;
	_indent_lvl = 0;

	std::string	directives[] = {"root", "index", "methods",
									"return", "CGI", "client_max_body_size",
									"upload_location", "cgi_extension", "autoindex"};
	typedef void (LocationConfig::*LocationConfigFunction)(tokeniterator begin, tokeniterator end);
	LocationConfigFunction functions[] = {&LocationConfig::validateRoot, 
											&LocationConfig::validateIndex, 
											&LocationConfig::validateMethods, 
											&LocationConfig::validateRedirect, 
											&LocationConfig::validateCGI, 
											&LocationConfig::validateBodySize, 
											&LocationConfig::validateUploadLocation, 
											&LocationConfig::validateCGIExtension,
											&LocationConfig::validateAutoindex};
	int size = sizeof(directives) / sizeof(directives[0]);
	for (int i = 0; i < size; i++){
		_directives_set[directives[i]] = false;
		_directives_validation_funcs[directives[i]] = functions[i];
	}
}

LocationConfig::LocationConfig(){
	init();
}

LocationConfig::LocationConfig(std::string root){
	_root = root;	
}

LocationConfig::LocationConfig(LocationConfig const & origin){
	*this = origin;		
}

LocationConfig & LocationConfig::operator=(LocationConfig const & origin){
	if (this == &origin)
		return *this;

	_root = origin._root; 
	_index = origin._index;
	_methods_allowed = origin._methods_allowed;
	_redirect = origin._redirect;
	_CGI = origin._CGI;
	_max_body_size = origin._max_body_size;
	_upload_location = origin._upload_location;
	_cgi_extension = origin._cgi_extension;
	_allow_get = origin._allow_get;
	_allow_post = origin._allow_post;
	_autoindex = origin._autoindex;
	_locations = origin._locations;
	_indent_lvl = origin._indent_lvl;
	_directives_set = origin._directives_set; 
	_directives_validation_funcs = origin._directives_validation_funcs;

	return (*this);
}

LocationConfig::~LocationConfig(){
}

std::string	LocationConfig::getIndex() const{
	return (_index);
}

/* bool LocationConfig::getIndex(std::string index){
	return (_);
} */

std::set<std::string> LocationConfig::getMethods() const{
	if (_methods_allowed.empty())
		return std::set<std::string>();
	std::set<std::string>	methods;
	methods = _methods_allowed;
	return (methods);
}

bool LocationConfig::hasMethod(std::string method){
	if (_methods_allowed.find(method) != _methods_allowed.end())
		return true;
	return false;
}

std::string		LocationConfig::getRoot() const {
	return _root;
}


std::pair<int, std::string>	LocationConfig::getRedirect() const{
	return _redirect;
}

std::string	LocationConfig::getCGI() const{
	return _CGI;
}

size_t LocationConfig::getBodySize() const{
	return _max_body_size;	
}

std::string LocationConfig::getUploadLocation() const{
	return _upload_location;
}

std::string LocationConfig::getCGIExtension() const{
	return _cgi_extension;
}

std::map<std::string, LocationConfig> const LocationConfig::getLocations() const{
	if (_locations.empty())
		return std::map<std::string, LocationConfig>();
	std::map<std::string, LocationConfig>	locations;
	locations = _locations;
	return (locations);
}

bool LocationConfig::getAllowGET() const{
	if (_methods_allowed.find("GET") != _methods_allowed.end())
		return true;
	return false;
}

bool LocationConfig::getAllowPOST() const{
	if (_methods_allowed.find("POST") != _methods_allowed.end())
		return true;
	return false;
}

bool LocationConfig::getAllowDELETE() const{
	if (_methods_allowed.find("DELETE") != _methods_allowed.end())
		return true;
	return false;
}

bool LocationConfig::getAutoindex() const {
	return _autoindex;
}

size_t LocationConfig::getIndent() const {
	return _indent_lvl;
}

void	LocationConfig::set_indent(size_t new_level){
	_indent_lvl = new_level;
}

void LocationConfig::parseLocationDirective(tokeniterator begin, tokeniterator end) {
	while (*end == "")
		--end;
	// init();
	if (_directives_set.find(*begin) == _directives_set.end()) {
		throw std::invalid_argument("invalid directive: " + *begin);
	}
	std::map<std::string, void (LocationConfig::*)(tokeniterator, tokeniterator)>::iterator function = _directives_validation_funcs.find(*begin);
	if (function != _directives_validation_funcs.end()) {
		try {
			(this->*(function->second))(begin + 1, end);
		}
		catch(const std::exception& e) {
			throw std::invalid_argument("directive " + *begin + ": " + e.what());
		}
	}
}

void LocationConfig::validateRoot(tokeniterator begin, tokeniterator end){
	
	if (begin == end){
		if (directoryExists(*begin)) {
			if ((*_directives_set.find("root")).second)
				std::cerr << COLOR_WARNING << "Warning: multiple root directives. using last." << COLOR_STANDARD << std::endl;
			_root = (*begin);
			_directives_set["root"] = true;
		}
		else
			throw std::invalid_argument("invalid parameter: " + *begin);
	}
	else
		throw std::invalid_argument("invalid parameter: " + *begin);
}

void LocationConfig::validateIndex(tokeniterator begin, tokeniterator end){
	
	if (begin != end)
		throw std::invalid_argument("invalid parameter: " + *begin);

	if ((*_directives_set.find("index")).second)
		std::cerr << COLOR_WARNING << "Warning: multiple index directives. using last." << COLOR_STANDARD << std::endl;

	_index = *begin;
}

void LocationConfig::validateMethods(tokeniterator begin, tokeniterator end){

	while (begin <= end){
		if (!hasMethod(*begin) && 
			(*begin == "GET" || *begin == "POST" || *begin == "DELETE"))
			_methods_allowed.insert(*begin);
		else if (!hasMethod(*begin))
			throw std::invalid_argument("invalid parameter: " + *begin);
		begin++;
	}
}

void LocationConfig::validateRedirect(tokeniterator begin, tokeniterator end){
	
	if (begin + 1 == end){
		int statusCode = std::atoi((*begin).c_str());
		if (statusCode < 100 || statusCode > 599)
			throw std::invalid_argument("invalid parameter: " + *begin);
		if (statusCode != 301 && statusCode != 302 &&
				statusCode != 307 && statusCode != 308)
			std::cerr << "Warning: unusual status code for redirect: " << statusCode << std::endl;
		if (++begin != end)
			throw std::invalid_argument("invalid parameter: " + *begin);
		if ((*_directives_set.find("return")).second)
			std::cerr << COLOR_WARNING << "Warning: Multiple return directives. Will use last." << COLOR_STANDARD << std::endl;
		_redirect.first = statusCode;
		_redirect.second = *begin;
		_directives_set["return"] = true;
	}
	else
		throw std::invalid_argument("Invalid return arguments.");
}

void LocationConfig::validateCGI(tokeniterator begin, tokeniterator end){
	if (begin == end){
		if (directoryExists(*begin)) {
			_CGI = (*begin);
		}
		else
			throw std::invalid_argument("invalid parameter: " + *begin);
	}
	else
		throw std::invalid_argument("invalid number of parameters.");
}

void LocationConfig::validateBodySize(tokeniterator begin, tokeniterator end){

	if (begin == end){
		char* endptrx = NULL;
		long double body = strtod((*begin).c_str(), &endptrx);
		std::string endptr = endptrx;

		if (body < 0)
			throw std::invalid_argument("invalid parameter: " + *begin);
		if (endptr == "" || endptr == "B")
			_max_body_size = body;
		else if (endptr == "M" || endptr == "MB")
			_max_body_size = body * 1024;
		else if (endptr == "M" || endptr == "MB")
			_max_body_size = body * 1024 * 1024;
		else if (endptr == "G" || endptr == "GB")
			_max_body_size = body * 1024 * 1024 * 1024;
		else
			throw std::invalid_argument("invalid parameter: " + *begin);
	}
	else
		throw std::invalid_argument("invalid number of parameters");
	if ((*_directives_set.find("client_max_body_size")).second){
		std::cerr << COLOR_WARNING << "Warning: Multiple client_max_body_size directives. Will use last." << COLOR_STANDARD << std::endl;
		_directives_set["client_max_body_size"] = true;
	}
}

void LocationConfig::validateUploadLocation(tokeniterator begin, tokeniterator end){
	if (begin == end){
		if (directoryExists(*begin)) {
			_upload_location = (*begin);
		}
		else
			throw std::invalid_argument("invalid parameter: " + *begin);
	}
	else
		throw std::invalid_argument("invalid number of parameters.");
}

void LocationConfig::validateCGIExtension(tokeniterator begin, tokeniterator end){
	if (begin == end)
		_cgi_extension = *begin;
}

void LocationConfig::validateAutoindex(tokeniterator begin, tokeniterator end){	
	if (begin == end){
		if (*begin == "1" || *begin == "on")
			_autoindex = true;
		else if (*begin == "0" || *begin == "off")
			_autoindex = false;
		else
			throw std::invalid_argument("invalid parameter: " + *begin);
	}
	else
		throw std::invalid_argument("invalid number of parameters.");
}

LocationConfig::LocationConfig(tokeniterator begin, tokeniterator end){
	tokeniterator statementend;
	init();
	try	{
		while (begin < end){
			while (*begin == "")
				begin++;
			if (*begin == "location"){
				begin++;
				while (*begin == "")
					begin++;
				std::string location_name = *begin++;
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
					begin = statementend + 1;
				}
				else
					throw std::invalid_argument("location block " + *begin + " missing closing '}'");
			}
			else if ((statementend = std::find(begin, end + 1, ";")) <= end) {
				parseLocationDirective(begin, statementend - 1);
				begin = statementend + 1;
			}
			else
				throw std::invalid_argument("directive: " + *begin + " missing closing ';'.");
		}
	}
	catch(const std::exception& e)	{
			throw std::invalid_argument(e.what());
		}
}

std::ostream& operator<<(std::ostream& os, const LocationConfig& locationconf) {
	// os << "++START LOCATION CONFIGURATION++" << std::endl << std::endl;

	std::string indent = "\t\t";
	for (int lvl = locationconf.getIndent(); lvl >= 0; lvl--)
		indent += "\t";

	std::vector<std::string> printbuff;
	os <<  indent << "root\t\t\t" << locationconf.getRoot() << ";" << std::endl;

	os <<  indent << "index\t\t\t" << locationconf.getIndex() << ";" << std::endl;
	
	os <<  indent << "methods\t\t\t";
	std::set<std::string> printset = locationconf.getMethods();
	for (std::set<std::string>::iterator it = printset.begin(); it != printset.end(); it++){
		os << *it;
  		std::set<std::string>::iterator next_it = it;
		++next_it;
		if (next_it != printset.end())
			os << " ";
	}
	os << ";" << std::endl;

	os <<  indent << "return\t\t\t" << locationconf.getRedirect().first << " " << locationconf.getRedirect().second << ";" << std::endl;
	os <<  indent << "CGI\t\t\t" << locationconf.getCGI() << ";" << std::endl;
	os <<  indent << "client_max_body_size\t" << locationconf.getBodySize() << ";" << std::endl;
	os <<  indent << "upload location\t\t" << locationconf.getUploadLocation() << ";" << std::endl;
	os <<  indent << "CGI extension\t\t" << locationconf.getCGIExtension() << ";" << std::endl;
	os <<  indent << "autoindex\t\t" << locationconf.getAutoindex() << ";" << std::endl;
	std::map<std::string, LocationConfig> locations = locationconf.getLocations();
	for (std::map<std::string, LocationConfig>::iterator it = locations.begin(); it != locations.end(); it++){
		(*it).second.set_indent(locationconf.getIndent() + 1);
		os <<  indent << "location\t\t" << (*it).first << " {" << std::endl 
			<< ((*it).second) << "" << std::endl;
		}
	os << indent << "}";
	return os;
}