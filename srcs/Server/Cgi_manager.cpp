/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi_manager.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmauguin <fmauguin@student.42.fr >         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/25 11:55:51 by fmauguin          #+#    #+#             */
/*   Updated: 2022/12/05 14:34:42 by fmauguin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>

#include <unistd.h>
#include <algorithm>
#include <iostream>
#include <fstream>


#include "Cgi_manager.hpp"
#include "utils.hpp"

extern int flags;

#ifndef __GNUC__
#pragma region Constructor &&Destructor
#endif

Cgi_manager::Cgi_manager(Request *request, Setup *setup, std::string const& client_ip, std::string const& cgi_exe, WebServer& webserv) :
	_request(request),
	_setup(setup),
	_client_ip(client_ip),
    _cgi_exe(cgi_exe),
	_webserv(webserv)
{
    this->_content_type = request->getField("Content-Type");
    if (this->_content_type == "")
        {this->_content_type = "text/plain";}
	this->_init();
	return;
}

Cgi_manager::Cgi_manager(Cgi_manager const &src): _webserv(src._webserv)
{
	*this = src;
	return;
}

Cgi_manager::~Cgi_manager(void)
{
}

#ifndef __GNUC__
#pragma endregion Constructor &&Destructor
#endif

Cgi_manager &Cgi_manager::operator=(Cgi_manager const &rhs)
{
	if (this != &rhs)
	{
		this->_request = rhs.getRequest();
		this->_setup = rhs.getSetup();
	}
	return *this;
}

Request *Cgi_manager::getRequest(void) const
{
	return this->_request;
}

Setup *Cgi_manager::getSetup(void) const
{
	return this->_setup;
}
std::ostream &operator<<(std::ostream &o, Cgi_manager const &rhs)
{
	(void)rhs;
	o << "Cgi manager" << std::endl;
	return o;
}

void Cgi_manager::_init(void)
{
    this->_env["REDIRECT_STATUS"] = "200";
    this->_env["GATEWAY_INTERFACE"] = "CGI/1.1";
    this->_env["SERVER_PROTOCOL"] = "HTTP/1.1";
    this->_env["SERVER_SOFTWARE"] = "WEBSERV/1.0";
    this->_env["REQUEST_METHOD"] = this->_request->getMethod();
    this->_env["CONTENT_TYPE"] = _content_type;                        // MIME TYPE, null if not known
    this->_env["CONTENT_LENGTH"] = this->_request->getField("Content-Length"); // If content is empty ==>
    if (this->_env["CONTENT_LENGTH"] == "")
        this->_env["CONTENT_LENGTH"] = "0";
    this->_env["PATH_INFO"] = this->_setup->getUri().substr(this->_request->getLocation()->getRoot().size());                        // cgi file
    this->_env["PATH_TRANSLATED"] = this->_setup->getUri();                    // cgi file
    this->_env["REQUEST_URI"] = this->_setup->getUri();                        // cgi file
    this->_env["QUERY_STRING"] = this->_setup->getQuery();
    this->_env["SCRIPT_NAME"] = this->_cgi_exe;       // cgi binary

    if (flags & FLAG_VERBOSE)
        std::cerr << "[ CGI uri: " << this->_setup->getUri() << " ]" << std::endl;

    this->_env["SCRIPT_FILENAME"] = this->_setup->getUri(); // full pathname

     this->_env["REMOTE_ADDR"] = this->_client_ip; // Addr remote get from socket
     this->_env["REMOTE_HOST"] = this->_client_ip; // Addr remote get from socket

    //if AUTH
    // this->_env["REMOTE_IDENT"] = _request->getClient()->getLogin(); // Need class CLIENT link request and socket
    // this->_env["REMOTE_USER"] = this->_env["REMOTE_IDENT"];

    this->_env["SERVER_NAME"] = _setup->getServer()->getServerName(); // get the good NODE from ConfigureTree
    this->_env["SERVER_PORT"] = _setup->getServer()->getPort(); // get the good NODE from ConfigureTree

    std::string tmp;
    for (std::map<std::string, std::string>::const_iterator it = this->_request->getFields().begin(); it != this->_request->getFields().end(); it++)
    {
        tmp = "HTTP_" + it->first;
        std::transform(tmp.begin(), tmp.end(), tmp.begin(), myToUpper);
        this->_env[tmp.c_str()] = it->second;
    }
}


char *convert(const std::string &s)
{
	char *pc = new char[s.size() + 1];
	std::strcpy(pc, s.c_str());
	return pc;
}

int Cgi_manager::execute(int *cgi_fd)
{
    char *argv[3];
    int fd_pipe[2];
    std::string tmp;

    std::cout << "======================CGI EXECUTE===========================" << std::endl;
    char **env = new char*[_env.size() + 1];
    int i = 0;
    std::string *env_arr = new std::string[_env.size() + 1];
    for (std::map<std::string, std::string>::iterator it = this->_env.begin(); it != _env.end(); it++)
    {
        tmp = it->first + "=" + it->second;
        env_arr[i] = it->first + "=" + it->second;
        env[i] = &env_arr[i][0];
        i++;
    }
    env[i] = NULL;
    argv[0] = &_env["SCRIPT_NAME"][0];

    if (flags & FLAG_VERBOSE)
        std::cerr << "[ Execute CGI: " << argv[0] << " ]" << std::endl;
    argv[1] = &_env["SCRIPT_FILENAME"][0];
    argv[2] = NULL;

    if (pipe(fd_pipe) != 0)
        return (500);
    *cgi_fd = fd_pipe[0]; //j'imagine que c ca qu'il faut faire
    pid_t pid = fork();
    std::cerr << argv[0] << std::endl;
    std::cerr << argv[1] << std::endl;
    std::cerr << argv[2] << std::endl;
    if (pid == 0)
    {
		this->_webserv.end();
        //0 = read, 1 = write
        if (dup2(fd_pipe[0], 0) == -1)
            return (500);
        write(fd_pipe[1], _request->getBody().c_str(), _request->getBody().size());
        if (dup2(fd_pipe[1], 1) == -1)
            return (500);
        close(fd_pipe[0]);
        close(fd_pipe[1]);
        std::cerr << "[ EXECVE ]" << std::endl;
        execve(argv[0], argv, env);
        std::cerr << "[ EXECVE FAIL ]" << std::endl;

        delete[] env;
        delete[] env_arr;
        exit(1);
    }
    else if (pid < 0)
        return 500;
    close(fd_pipe[1]);
    delete[] env;
    delete[] env_arr;

    return 0;
}
