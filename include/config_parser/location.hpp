/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/04 16:55:56 by jschott           #+#    #+#             */
/*   Updated: 2024/04/05 10:13:41 by jschott          ###   ########.fr       */
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
	
public:
	location();
	location(location const & origin);
	location & operator=(location const & origin);
	~location();
};

#endif
