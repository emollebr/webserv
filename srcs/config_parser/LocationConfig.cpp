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
									"redirect", "CGI", "max_body",
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

void LocationConfig::parseLocationDirective(tokeniterator begin, tokeniterator end) {
	init();
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
	// std::cout << TEXT_BOLD << "validating root" << TEXT_NOFORMAT<< std::endl;
	
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

void LocationConfig::validateIndeces(tokeniterator begin, tokeniterator end){
	
	if ((*_directives_set.find("index")).second)
		std::cerr << COLOR_WARNING << "Warning: multiple index directives. using last." << COLOR_STANDARD << std::endl;

	while (begin <= end)
		_indeces.push_back(*begin++);
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
	if (begin == end)
		_CGI = *begin;
}

void LocationConfig::validateBodySize(tokeniterator begin, tokeniterator end){

	if (begin == end){
		char* endptrx = NULL;

		unsigned long int body = strtoul((*begin).c_str(), &endptrx, 0);
		std::string endptr = endptrx;

	// MISSING INTERNAL LIMIT FOR UPLOAD SIZE THAT MAY NOT BE EXCEEDED TBD

		if (endptr == "" || endptr == "B") {
			if ((*_directives_set.find("client_max_body_size")).second)
				std::cerr << COLOR_WARNING << "Warning: Multiple client_max_body_size directives. Will use last." << COLOR_STANDARD << std::endl;
			_max_body_size = body;
			_directives_set["client_max_body_size"] = true;
		}
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

void LocationConfig::validateAllowGET(tokeniterator begin, tokeniterator end){
	if (begin == end)
		return ;
}

void LocationConfig::validateAllowPOST(tokeniterator begin, tokeniterator end){
	if (begin == end)
		return ;
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
	tokeniterator directiveend;
	try	{
		while (begin < end){
			if ((directiveend = std::find(begin, end + 1, ";")) <= end) {
				parseLocationDirective(begin, directiveend - 1);
				begin = directiveend + 1;
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
	std::vector<std::string> printbuff;
	os << "\t\troot\t\t\t" << locationconf.getRoot() << ";" << std::endl;

	os << "\t\tindex\t\t\t";
	printbuff = locationconf.getIndeces();
	for (std::vector<std::string>::iterator it = printbuff.begin(); it != printbuff.end(); it++){
		os << *it;
		if (it + 1 != printbuff.end())
			os << " ";
	}
	os << ";" << std::endl;
	
	os << "\t\tmethods\t\t\t";
	std::set<std::string> printset = locationconf.getMethods();
	for (std::set<std::string>::iterator it = printset.begin(); it != printset.end(); it++){
		os << *it;
  		std::set<std::string>::iterator next_it = it;
		++next_it;
		if (next_it != printset.end())
			os << " ";
	}
	os << ";" << std::endl;

	os << "\t\treturn\t\t\t" << locationconf.getRedirect().first << " " << locationconf.getRedirect().second << ";" << std::endl;
	os << "\t\tCGI\t\t\t" << locationconf.getCGI() << ";" << std::endl;
	os << "\t\tclient_max_body_size\t" << locationconf.getBodySize() << ";" << std::endl;
	os << "\t\tupload location\t\t" << locationconf.getUploadLocation() << ";" << std::endl;
	os << "\t\tCGI extension\t\t" << locationconf.getCGIExtension() << ";" << std::endl;
	os << "\t\tautoindex\t\t" << locationconf.getAutoindex() << ";" << std::endl;
	os << "	}";
	return os;
}
