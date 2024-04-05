/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/04 16:51:38 by jschott           #+#    #+#             */
/*   Updated: 2024/04/05 12:21:20 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include <string>
#include <map>
#include <vector>
#include "location.hpp"

class server
{
private:
	std::vector<size_t>						_listen; // ports 0-1023 well-known | 1024-49151 registered | 49152-65535 dynamic & private port
	std::map<std::string, location*> const	_locations;
	std::string								_host;

//OPTIONAL INFORMATION
	std::string const						_server_name;
	std::string	const						_error_path;

	server();


public:
	server(std::vector<size_t> ports);
	server(server const & origin);
	server(std::string name, std::vector<int> listen, std::string err, location location);
	server & operator= (server const & origin);
	~server();

	void 				addLocation(location location);
	void				setErrorPath(std::string error_path);
	void				setServerName(std::string server_name);

	std::vector<size_t>							getListenPorts();
	std::map<std::string, location> const		getLocations();
	location const								getLocation(std::string directory);
	std::string const							getServerName();
	std::string	const							getErrorPath();
};

const server*	parseServer(std::string serverConfig);

#endif 