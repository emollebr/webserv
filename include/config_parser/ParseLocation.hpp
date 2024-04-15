/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseLocation.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/04 16:55:56 by jschott           #+#    #+#             */
/*   Updated: 2024/04/15 16:34:04 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSELOCATION_HPP
# define PARSELOCATION_HPP

#include <string>
#include <vector>
#include <deque>
#include "ReadConfig.hpp"

class ParseLocation
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
	
	/* std::vector<std::string>	_directives = {"root", "index", "methods_allowed",
												"redirect", "CGI", "max_body_size",
												"default_file", "upload_location", 
												"cgi_extension", "allow_get", "allow_post"};
 */
	ParseLocation();

public:
	ParseLocation(std::string root);
	ParseLocation(std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);
	ParseLocation(ParseLocation const & origin);
	ParseLocation & operator=(ParseLocation const & origin);
	~ParseLocation();

	std::vector<std::string>	getIndeces();
	std::vector<std::string>	getMethods();
	std::string					getRedirect();
	std::string					getCGI();
	size_t						getBodySize();
	std::string					getDefaultFile();
	std::string					getUploadLocation();
	std::string					getCGIExtension();
	bool						getAllowGET();
	bool						getAllowPOST();

	bool						hasIndex(std::string index);
	bool						hasMethod(std::string method);
	
};

#endif
