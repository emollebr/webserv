/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/04 16:55:56 by jschott           #+#    #+#             */
/*   Updated: 2024/04/17 15:57:53 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

#include <string>
#include <vector>
#include <deque>
#include "ReadConfig.hpp"

class LocationConfig
{
private:
	std::string					_root; // /var/ww/html
	
// OPTIONAL VALUES
	std::vector<std::string>	_indeces; // index.html index.php
	std::vector<std::string>	_methods_allowed; // GET POST DELETE
	std::string					_redirect; // /new-location
	std::string					_CGI; // /cgi_bin/script
	size_t						_max_body_size;
	std::string					_default_file; // index.html
	std::string					_upload_location; // /uploads
	std::string					_cgi_extension; // .php
	bool						_allow_get;
	bool						_allow_post;
	bool						_autoindex;
	

	std::map<std::string, bool>	_directives_index;
	std::map<std::string, void (LocationConfig::*)(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end)> _validation_index;

	void						init();

public:
	LocationConfig();
	LocationConfig(std::string root);
	LocationConfig(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	LocationConfig(LocationConfig const & origin);
	LocationConfig & operator=(LocationConfig const & origin);
	~LocationConfig();

	std::string					getRoot() const;
	std::vector<std::string>	getIndeces() const;
	std::vector<std::string>	getMethods() const;
	std::string					getRedirect() const;
	std::string					getCGI() const;
	size_t						getBodySize() const;
	std::string					getDefaultFile() const;
	std::string					getUploadLocation() const;
	std::string					getCGIExtension() const;
	bool						getAllowGET() const;
	bool						getAllowPOST() const;
	bool						getAutoindex() const;

	void						parseLocationDirective(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void						validateRoot(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void						validateIndeces(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void						validateMethods(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void						validateRedirect(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void						validateCGI(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void						validateBodySize(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void						validateDefaultFile(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void						validateUploadLocation(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void						validateCGIExtension(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void						validateAllowGET(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void						validateAllowPOST(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	void						validateAutoindex(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);

	bool						hasIndex(std::string index);
	bool						hasMethod(std::string method);
};

std::ostream& operator<<(std::ostream& os, const LocationConfig& locationconf);

#endif
