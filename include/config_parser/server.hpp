/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/04 16:51:38 by jschott           #+#    #+#             */
/*   Updated: 2024/04/04 17:39:09 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <map>
#include <vector>
#include "location.hpp"

class server
{
private:
	std::string const						_server_name;
	std::vector<int const>					_listen;
	std::string	const						_error_path;
	std::map<std::string, location> const	_locations;

public:
	server();
	server(server const & origin);
	server(std::string name, std::vector<int> listen, std::string err, location location);
	server & operator= (server const & origin);
	~server();
};

server	parseServer(std::string serverConfig);