/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi_manager.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/01 04:57:35 by fmauguin          #+#    #+#             */
/*   Updated: 2022/12/02 18:31:10 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_MANAGER_HPP
#define CGI_MANAGER_HPP

#include <iostream>
#include "Request.hpp"
#include "WebServer.hpp"
#include "Configure.hpp"

// Parse Request
// Set ENV
// PIPE + FORK
//		CHANGE DIR ?
//		EXECVE
// TRANSFER STDOUT

class Cgi_manager
{
private:
	Request *_request;
	Setup *_setup;
	std::map<std::string, std::string> _env;
	std::string _response;
	std::string _content_type;
	std::string _client_ip;
	std::string _cgi_exe;
	WebServer&	_webserv;
	//int			_file_fd;
	void _init(void);

public:
	Cgi_manager(Request *request, Setup *setup, std::string const& client_ip, std::string const& cgi_exe, WebServer&);
	Cgi_manager(Cgi_manager const &src);
	~Cgi_manager();

	Request *getRequest(void) const;
	Setup *getSetup(void) const;
	int execute(int *cgi_fd);
	Cgi_manager &operator=(Cgi_manager const &rhs);
};

std::ostream &operator<<(std::ostream &o, Cgi_manager const &rhs);

#endif
