/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/04 16:55:56 by jschott           #+#    #+#             */
/*   Updated: 2024/04/10 16:23:58 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

#include <string>
#include <vector>

class location
{
private:
	std::string					_root; // /var/ww/html
	
// OPTIONAL VALUES
	std::vector<std::string>	_index; // index.html index.php
	std::vector<std::string>	_methods_allowed; // GET POST DELETE
	std::string					_redirect; // /new-location
	std::string					_CGI; // /cgi_bin/script
	size_t						_max_body_size;
	std::string					_default_file; // index.html
	std::string					_upload_location; // /uploads
	std::string					_cgi_extension; // .php
	bool						_allow_get;
	bool						_allow_post;
	
	std::vector<std::string>	_directives = {"root", "index", "methods_allowed",
												"redirect", "CGI", "max_body_size",
												"default_file", "upload_location", 
												"cgi_extension", "allow_get", "allow_post"};

	location();

public:
	location(std::string root);
	location(location const & origin);
	location & operator=(location const & origin);
	~location();

	std::vector<std::string>	getIndex();
	bool						getIndex(std::string index);
	std::vector<std::string>	getMethods();
	bool						getMethods(std::string method);
	std::string					getredirect();
	std::string					getCGI();
	size_t						getBodySize();
	std::string					getDefaultFile();
	std::string					getUploadLocation();
	std::string					getCGIExtension();
	bool						getAllowGET();
	bool						getAllowPOST();
	
};

const location*	parseLocationBlock(std::string config, int const line);

#endif
