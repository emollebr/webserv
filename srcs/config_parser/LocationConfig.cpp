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
	std::string	directives[] = {"root", "index", "methods",
									"redirect", "CGI", "client_max_body_size",
									"default_file", "upload_location", 
									"cgi_extension", "allow_get", "allow_post", 
									"autoindex"};
	typedef void (LocationConfig::*LocationConfigFunction)(tokeniterator begin, tokeniterator end);
	LocationConfigFunction functions[] = {&LocationConfig::validateRoot, &LocationConfig::validateIndeces, &LocationConfig::validateMethods, 
											&LocationConfig::validateRedirect, &LocationConfig::validateCGI, &LocationConfig::validateBodySize, 
											&LocationConfig::validateDefaultFile, &LocationConfig::validateUploadLocation, &LocationConfig::validateCGIExtension, 
											&LocationConfig::validateAllowGET, &LocationConfig::validateAllowPOST, &LocationConfig::validateAutoindex};
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
	_indeces = origin._indeces;
	_methods_allowed = origin._methods_allowed;
	_redirect = origin._redirect;
	_CGI = origin._CGI;
	_max_body_size = origin._max_body_size;
	_default_file = origin._default_file;
	_upload_location = origin._upload_location;
	_cgi_extension = origin._cgi_extension;
	_allow_get = origin._allow_get;
	_allow_get = origin._allow_get;
	return (*this);
}

LocationConfig::~LocationConfig(){
}

std::vector<std::string>	LocationConfig::getIndeces() const{
	if (_indeces.empty())
		return std::vector<std::string>();
	std::vector<std::string>	Indeces;
	Indeces = _indeces;
	return (Indeces);
}

/* bool LocationConfig::getIndex(std::string index){
	return (_);
} */

std::vector<std::string> LocationConfig::getMethods() const{
	if (_methods_allowed.empty())
		return std::vector<std::string>();
	std::vector<std::string>	methods;
	methods = _methods_allowed;
	return (methods);
}

bool LocationConfig::hasMethod(std::string method){
	if (std::find(_methods_allowed.begin(), _methods_allowed.end(), method) != _methods_allowed.end())
		return true;
	return false;
}

std::string		LocationConfig::getRoot() const {
	return _root;
}


std::string	LocationConfig::getRedirect() const{
	return _redirect;
}

std::string	LocationConfig::getCGI() const{
	return _CGI;
}

size_t LocationConfig::getBodySize() const{
	return _max_body_size;	
}

std::string LocationConfig::getDefaultFile() const{
	return _default_file;
}

std::string LocationConfig::getUploadLocation() const{
	return _upload_location;
}

std::string LocationConfig::getCGIExtension() const{
	return _cgi_extension;
}

bool LocationConfig::getAllowGET() const{
	return _allow_get;
}

bool LocationConfig::getAllowPOST() const{
	return _allow_post;
}

bool LocationConfig::getAutoindex() const {
	return _autoindex;
}

void LocationConfig::parseLocationDirective(tokeniterator begin, tokeniterator end) {
	init();
	if (_directives_set.find(*begin) == _directives_set.end()) {
		return ; //THROW EXCEPTION
	}
	std::map<std::string, void (LocationConfig::*)(tokeniterator, tokeniterator)>::iterator function = _directives_validation_funcs.find(*begin);
	if (function != _directives_validation_funcs.end()) {
		try {
			(this->*(function->second))(begin + 1, end);
		}
		catch(const std::exception& e) {
			throw InvalidConfigException();
			std::cerr << e.what() << std::endl;
		}
	}
}

void LocationConfig::validateRoot(tokeniterator begin, tokeniterator end){
	std::cout << TEXT_BOLD << "validating root" << TEXT_NOFORMAT<< std::endl;
	if (begin == end){
		// if (stat((*begin).c_str(), &info) == 0)
		if (directoryExists(*begin))
			_root = (*begin);
		else
			throw InvalidConfigException();
			// std::cerr << "Error: root directory " << *begin << " does not exist";
	}
	else
		throw InvalidConfigException();
		// std::cerr << "Error: Invalid Root Directory" << std::endl;
}

void LocationConfig::validateIndeces(tokeniterator begin, tokeniterator end){
	// if (begin == end)
	// 	return ;
	while (begin <= end)
		_indeces.push_back(*begin++);
}

void LocationConfig::validateMethods(tokeniterator begin, tokeniterator end){
	std::cout << TEXT_BOLD << "validating methods" << TEXT_NOFORMAT<< std::endl;

	while (begin <= end){
		std::cout << "checking " << *begin << std::endl;
		if (!hasMethod(*begin) && 
			(*begin == "GET" || *begin == "POST" || *begin == "DELETE"))
			_methods_allowed.push_back(*begin);
		else if (!hasMethod(*begin))
			throw InvalidConfigException();
		begin++;
	}
}

void LocationConfig::validateRedirect(tokeniterator begin, tokeniterator end){
	if (begin == end)
		_redirect = *begin;
}

void LocationConfig::validateCGI(tokeniterator begin, tokeniterator end){
	if (begin == end)
		_CGI = *begin;
}

void LocationConfig::validateBodySize(tokeniterator begin, tokeniterator end){
	std::cout << TEXT_BOLD << "validating body size" << TEXT_NOFORMAT<< std::endl;

	if (begin == end){
		// _max_body_size = atoi((*begin).c_str());
		char* endptrx = NULL;

		unsigned long int body = strtoul((*begin).c_str(), &endptrx, 0);
		std::string endptr = endptrx;

	// std::cout << "\tbody size:\t" << body << std::endl << "\tfailure:\t" << endptr << std::endl;

	// MISSING INTERNAL LIMIT FOR UPLOAD SIZE THAT MAY NOT BE EXCEEDED TBD

		if (endptr == "" || endptr == "B")
			_max_body_size = body;
		else if (endptr == "M" || endptr == "MB")
			_max_body_size = body * 1024;
		else if (endptr == "M" || endptr == "MB")
			_max_body_size = body * 1024 * 1024;
		else if (endptr == "G" || endptr == "GB")
			_max_body_size = body * 1024 * 1024 * 1024;
		else
			throw InvalidConfigException();
	}
	else
		throw InvalidConfigException();
}

void LocationConfig::validateDefaultFile(tokeniterator begin, tokeniterator end){
	if (begin == end)
		_default_file = *begin;
}

void LocationConfig::validateUploadLocation(tokeniterator begin, tokeniterator end){
	std::cout << TEXT_BOLD << "validating upload" << TEXT_NOFORMAT<< std::endl;
	if (begin == end){
		// if (stat((*begin).c_str(), &info) == 0)
		if (directoryExists(*begin))
			_root = (*begin);
		else
			throw InvalidConfigException();
			// std::cerr << "Error: root directory " << *begin << " does not exist";
	}
	else
		throw InvalidConfigException();
		// std::cerr << "Error: Invalid Root Directory" << std::endl;
}

void LocationConfig::validateCGIExtension(tokeniterator begin, tokeniterator end){
	if (begin == end)
		_cgi_extension = *begin;
}

void LocationConfig::validateAllowGET(tokeniterator begin, tokeniterator end){
	if (begin == end)
		return ;
}

void LocationConfig::validateAllowPOST(tokeniterator begin, tokeniterator end){
	if (begin == end)
		return ;
}

void LocationConfig::validateAutoindex(tokeniterator begin, tokeniterator end){
	std::cout << "validation autoindex: " << *begin << std::endl;
	
	if (begin == end){
		if (*begin == "1" || *begin == "on")
			std::cout << "THIS" << std::endl;
			// _autoindex = true;
		else if (*begin == "0" || *begin == "off")
			_autoindex = false;
		else
			throw InvalidConfigException();
	}
	else
		throw InvalidConfigException();
}

LocationConfig::LocationConfig(tokeniterator begin, tokeniterator end){
	// std::cout << TEXT_BOLD << "	Parsing Location from " << *begin << " to " << *end << std::endl;
	// tokeniterator directiveend = std::find(begin, end, ";") ;
	//IF FOUND ; CREATE TUPLE
	// try	{
		while (begin < end){
			tokeniterator directiveend = std::find(begin, end, ";") ;
			if (directiveend <= end) {
				parseLocationDirective(begin, directiveend - 1);
				begin = directiveend + 1;
			}
			else 
				std::cerr << COLOR_ERROR << "Error: Cannot parse directive" << std::endl << COLOR_STANDARD;
		}
	// }
	// catch(const std::exception& e)	{
		// throw InvalidConfigException();
		// std::cerr << e.what() << '\n';
		// }
	std::cout << TEXT_NOFORMAT;
}

std::ostream& operator<<(std::ostream& os, const LocationConfig& locationconf) {
	// os << "++START LOCATION CONFIGURATION++" << std::endl << std::endl;
	std::vector<std::string> printbuff;
	os << "\t\troot\t\t\t" << locationconf.getRoot() << ";" << std::endl;
	os << "\t\tindices\t\t\t" << "TBD" << ";" << std::endl;
	
	os << "\t\tmethods\t\t\t";
	printbuff = locationconf.getMethods();
	for (std::vector<std::string>::iterator it = printbuff.begin(); it != printbuff.end(); it++){
		os << *it;
		if (it + 1 != printbuff.end())
			os << " ";
	}
	os << ";" << std::endl;

	os << "\t\tredirect\t\t" << locationconf.getRedirect() << ";" << std::endl;
	os << "\t\tCGI\t\t\t" << locationconf.getCGI() << ";" << std::endl;
	os << "\t\tclient_max_body_size\t" << locationconf.getBodySize() << ";" << std::endl;
	os << "\t\tdefault file\t\t" << locationconf.getDefaultFile() << ";" << std::endl;
	os << "\t\tupload location\t\t" << locationconf.getUploadLocation() << ";" << std::endl;
	os << "\t\tCGI extension\t\t" << locationconf.getCGIExtension() << ";" << std::endl;
	os << "\t\tautoindex\t\t" << locationconf.getAutoindex() << ";" << std::endl;
	os << "	}";
	return os;
}
