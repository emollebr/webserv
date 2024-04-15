/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseServer.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/04 16:51:38 by jschott           #+#    #+#             */
/*   Updated: 2024/04/15 14:27:51 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSESERVER_HPP
# define PARSESERVER_HPP

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include "ParseLocation.hpp"
#include "ReadConfig.hpp"

class ParseServer
{
private:
	std::vector<size_t>						_listen; // ports 0-1023 well-known | 1024-49151 registered | 49152-65535 dynamic & private port
	std::map<std::string, ParseLocation*>	_locations;
	std::string								_host;
	

//OPTIONAL INFORMATION
	std::string						_server_name;
	std::string						_error_path;

	std::vector<std::string>	_directives = {"listen", "location", "host",
												 "host", "error_path"};

	ParseServer();


public:
	ParseServer(std::vector<size_t> ports);
	ParseServer(ParseServer const & origin);
	ParseServer(std::string name, std::vector<size_t> listen, std::string err, std::map<std::string, ParseLocation*> location);
	ParseServer & operator= (ParseServer const & origin);
	~ParseServer();

	void 				addLocation(std::pair<std::string, ParseLocation*> location);
	void				setErrorPath(std::string error_path);
	void				setServerName(std::string server_name);

	std::vector<size_t>							getListenPorts();
	std::map<std::string, ParseLocation*> const	getLocations();
	ParseLocation const							getLocation(std::string directory);
	std::string const							getServerName();
	std::string	const							getErrorPath();
};

const ParseServer*	parseServer(std::deque<std::string> tokens, std::deque<std::string>::iterator begin, std::deque<std::string>::iterator end);

#endif 