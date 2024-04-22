/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReadConfig.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jschott <jschott@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 14:23:34 by jschott           #+#    #+#             */
/*   Updated: 2024/04/18 10:56:53 by jschott          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#define TEXT_NOFORMAT	"\e[0m"
#define TEXT_BOLD		"\e[1m"
#define TEXT_UNDERLINE	"\e[4m"

#define COLOR_STANDARD	"\033[0m"
#define	COLOR_ERROR		"\033[31m"
#define COLOR_WARNING	"\033[33m"
#define COLOR_SUCCESS	"\033[92m"
#define COLOR_FOCUS		"\e[35m"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <stack>
#include <vector>
#include <set>
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
// #include <tuple>

typedef std::deque<std::string>::iterator tokeniterator;

void			parseDirective(tokeniterator begin, tokeniterator end);
bool 			isBalanced(std::stringstream& ss);
tokeniterator	getClosingBraket (std::deque<std::string>& queue, tokeniterator start);
void			printTokens(std::deque<std::string> tokens);
void			populateTokens(std::stringstream &bufferstream, std::deque<std::string>	&tokens);
void			readFile2Buffer (std::string filename);