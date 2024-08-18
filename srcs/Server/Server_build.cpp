/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_build.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmauguin <fmauguin@student.42.fr >         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/25 20:26:43 by lgiband           #+#    #+#             */
/*   Updated: 2022/12/05 14:46:28 by fmauguin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <cstring>

#include <sys/epoll.h>

#include "WebServer.hpp"

extern int flags;

int	WebServer::openFile(Setup *setup, Response *response)
{
	Cache			cache;
	std::ifstream	*stream;

	for (std::vector<Cache>::iterator it = this->_all_cache.begin(); it != this->_all_cache.end(); it++)
	{
		if (it->getUri() == setup->getUri())
		{
			it->setUsers(it->getUsers() + 1);
			response->setBodySize(it->getSize());
			response->setFilename(it->getUri());
			return (0);
		}
	}
	try {stream = new std::ifstream(setup->getUri().c_str());}
	catch (std::exception &e) {return (perror("/!\\ Open file failed"), setup->setServer(0),  setup->setCode(500), 500);}

	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Open file " << setup->getUri() << " ]" << std::endl;
	stream->seekg(0, stream->end);
	cache.setSize(stream->tellg());
	stream->seekg(0, stream->beg);
	if (cache.getSize() == std::string::npos)
	{
		if (flags & FLAG_VERBOSE)
			std::cerr << "/!\\ File size failed" << std::endl;
		delete stream;
		return (setup->setServer(0), setup->setCode(404), 404);
	}
	cache.setStream(stream);
	cache.setUri(setup->getUri());
	cache.setUsers(1);
	response->setFilename(setup->getUri());
	response->setBodySize(cache.getSize());
	
	this->_all_cache.push_back(cache);
	
	return (0);
}

int WebServer::buildResponseDefault(int client_fd, Request *request, Setup *setup)
{
	struct epoll_event	event;
	Response			response;
	std::string			status;
	
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Build response default ]" << std::endl;

	response.setFd(client_fd);
	response.setStatus(0);
	response.setPosition(0);
	if (setup->getServer() == 0)
	{
		if (this->_status_codes.find(setup->getCode()) != this->_status_codes.end()) //set dans le if apres
			status = this->_status_codes.find(setup->getCode())->second;
		else
			status = "Unknown";
		setup->setExtension(".html");
		response.setBody(setup->getCode(), status);
	}
	else
	{
		setup->setUri(setup->getCode());
		if (setup->getUri() == "")
			return (setup->setServer(0), buildResponseDefault(client_fd, request, setup));
		setup->setExtension();
		if (request->getLocation()->getCgiPerm().find(setup->getExtension()) != request->getLocation()->getCgiPerm().end())
			return (this->cgiMode(request, setup, client_fd));
		if (this->openFile(setup, &response)) 
			return (setup->setServer(0), buildResponseDefault(client_fd, request, setup));
		response.setFilename(setup->getUri());
	}
	response.setHeader(setup, this->_status_codes, this->_mimetypes, response.getBodySize());
	this->_all_response.push_back(response);
	
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Response Builded on fd " << client_fd << " ]" << std::endl;
	std::memset(&event, 0, sizeof(event));
	event.data.fd = client_fd;
 	event.events = EPOLLOUT;
 	epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, client_fd, &event);
	return (0);
}

int	WebServer::buildResponseListing(Request *request, Setup *setup, int client_fd)
{
	struct epoll_event	event;
	Response			response;

	(void)request;
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Build response listing ]" << std::endl;

	response.setFd(client_fd);
	response.setStatus(0);
	response.setPosition(0);
	if (response.setListingBody(setup->getUri(), setup->getServer()->getRoot()))
		return (setup->setCode(500), 500);

	setup->setExtension(".html");
	response.setHeader(setup, this->_status_codes, this->_mimetypes, response.getBodySize());
	
	this->_all_response.push_back(response);

	std::memset(&event, 0, sizeof(event));
	event.data.fd = client_fd;
 	event.events = EPOLLOUT;
 	epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, client_fd, &event);
	return (0);
}

int	WebServer::buildResponseGet(Request *request, Setup *setup, int client_fd)
{
	struct epoll_event	event;
	Response			response;

	(void)request;
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Build response Get ]" << std::endl;

	response.setFd(client_fd);
	response.setStatus(0);
	response.setPosition(0);
	if (this->openFile(setup, &response))
		return (500);

	setup->setExtension();
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Extension : " << setup->getExtension() << " ]" <<std::endl;
	response.setHeader(setup, this->_status_codes, this->_mimetypes, response.getBodySize());
	
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Header builded ]\n" << response.getHeader() << "[ End Header ]" << std::endl;
	
	this->_all_response.push_back(response);

	std::memset(&event, 0, sizeof(event));
	event.data.fd = client_fd;
 	event.events = EPOLLOUT;
 	epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, client_fd, &event);
	return (0);
}
