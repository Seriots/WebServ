/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_modeChoice.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmauguin <fmauguin@student.42.fr >         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/25 16:44:45 by lgiband           #+#    #+#             */
/*   Updated: 2022/12/05 14:41:58 by fmauguin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <cstring>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "WebServer.hpp"
#include "Cgi_manager.hpp"
#include "utils.hpp"

extern int flags;

int	WebServer::redirectMode(Request *request, Setup *setup, int client_fd)
{
	struct epoll_event event;
	Response response;

	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Build response Redirect ]" << std::endl;
	setup->setCode(301);

	setup->addField("Location", request->getLocation()->getRedirect());
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Redirect to " << request->getLocation()->getRedirect() << " ]" << std::endl;
	response.setFd(client_fd);
	response.setStatus(0);
	response.setPosition(0);
	response.setHeader(setup, this->_status_codes, this->_mimetypes, 0);

	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Header builded ]\n" << response.getHeader() << "[ End Header ]" << std::endl;

	this->_all_response.push_back(response);

	std::memset(&event, 0, sizeof(event));
	event.data.fd = client_fd;
	event.events = EPOLLOUT;
	epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, client_fd, &event);
	return (0);
}

int WebServer::cgiMode(Request *request, Setup *setup, int client_fd)
{
	Cgi_manager CgiManager(request, setup, this->_clientIP.find(client_fd)->second, request->getLocation()->getCgiPerm().find(setup->getExtensionName())->second, *this);
	Response	response;
	struct epoll_event	event;
	struct epoll_event	event2;
	int	cgi_fd;
	int ret;

	if (flags & FLAG_VERBOSE)
		std::cerr << "[ CGI mode ]" << std::endl;

	if (!doesPathExist(setup->getUri()))
		return (setup->setCode(404), 404);
	if (!isPathReadable(setup->getUri()))
		return (setup->setCode(403), 403);


	ret = CgiManager.execute(&cgi_fd);
	if (ret != 0)
		return (setup->setCode(ret), ret);
	this->_cgiFD.insert(std::make_pair(cgi_fd, client_fd));

	response.setFd(client_fd);
	response.setStatus(0);
	response.setPosition(0);
	response.setBodySize(-1);
	setup->setExtension(".html");

	response.setHeader("HTTP/1.1 200 OK\n\rConnection: close\r\n");

	this->_all_response.push_back(response);

	std::memset(&event, 0, sizeof(event));
	event.data.fd = cgi_fd;
	event.events = EPOLLIN;
	std::memset(&event2, 0, sizeof(event2));
	event2.data.fd = client_fd;
	event2.events = EPOLLOUT;
	epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, cgi_fd, &event);
	epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, client_fd, &event2);

	this->_timeout.insert( std::make_pair(cgi_fd, (t_pair){time(NULL), 3} ) );
	return (0);
}

int WebServer::getMode(Request *request, Setup *setup, int client_fd)
{
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Get Mode ]" << std::endl;

	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Uri : " << setup->getUri() << " ]" << std::endl;
	if (!doesPathExist(setup->getUri()))
		return (setup->setCode(404), 404);
	if (!isPathReadable(setup->getUri()))
		return (setup->setCode(403), 403);
	if (isDirectory(setup->getUri()) && request->getLocation()->getAutoindex() == false)
	{
		if (request->getLocation()->getDefaultFile() == "" || request->getLocation()->getDefaultFile() == setup->getUri())
			return (setup->setCode(404), 404);
		else
		{
			setup->setUri(request->getLocation()->getDefaultFile());
			setup->setExtension();
			if (request->getLocation()->getCgiPerm().find(setup->getExtension()) != request->getLocation()->getCgiPerm().end())
				return (this->cgiMode(request, setup, client_fd));
			else
				return (this->getMode(request, setup, client_fd));
		}
	}
	if (isDirectory(setup->getUri()) && request->getLocation()->getAutoindex() == true)
		return (setup->setCode(200), this->buildResponseListing(request, setup, client_fd));
	if (isFile(setup->getUri()))
		return (setup->setCode(200), this->buildResponseGet(request, setup, client_fd));
	return (setup->setCode(403), 403);
}

int WebServer::postMode(Request *request, Setup *setup, int client_fd)
{
	Response response;
	int ret;
	std::string field;

	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Post Mode ]" << std::endl;

	ret = this->setPostUri(request, setup);
	if (ret)
		return (this->sendPostResponse(request, setup, client_fd));
	ret = this->checkPostRequest(request, setup);
	if (ret)
		return (this->sendPostResponse(request, setup, client_fd));

	field = request->getField("Content-Type");

	if (field == "application/x-www-form-urlencoded")
		this->urlEncodedPost(request, setup);
	else if (field.find("multipart/form-data") != std::string::npos && field.find("boundary=") != std::string::npos)
		this->multipartPost(request, setup);
	else
		this->plainTextPost(request, setup);

	this->sendPostResponse(request, setup, client_fd);
	return (0);
}

int WebServer::deleteMode(Request *request, Setup *setup, int client_fd)
{
	Response response;
	int ret;

	(void)request;
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Delete Mode ]" << std::endl;

	ret = 1;
	if (doesPathExist(setup->getUri()) && isFile(setup->getUri()))
		ret = 0;

	if (ret == 0)
		ret = remove(setup->getUri().c_str());

	if (ret == 0)
	{
		setup->setCode(200);
		response.setBody("OK");
	}
	else
	{
		setup->setCode(400);
		response.setBody("KO");
	}
	response.setFd(client_fd);
	response.setStatus(0);
	response.setPosition(0);
	setup->setExtension("");
	response.setHeader(setup, this->_status_codes, this->_mimetypes, response.getBody().size());

	send(client_fd, response.getHeader().c_str(), response.getHeader().size(), MSG_NOSIGNAL | MSG_MORE);
	send(client_fd, response.getBody().c_str(), response.getBody().size(), MSG_NOSIGNAL);

	epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client_fd, 0);
	this->_timeout.erase(client_fd);
	close(client_fd);

	return (0);
}

int WebServer::modeChoice(Request *request, Setup *setup, int client_fd)
{
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Mode choice ]" << std::endl;

	if (request->getLocation()->getRedirect() != "" && !isMe(setup->getUri(), request->getLocation()->getRedirect(), setup->getServer()->getRoot()))
		return (this->redirectMode(request, setup, client_fd));

	if (request->getMethod() == "GET" && !(request->getLocation()->getPermission() & GET_PERM))
		return (derror("/!\\ GET not allowed"), setup->setCode(405), 405);
	if (request->getMethod() == "POST" && !(request->getLocation()->getPermission() & POST_PERM))
		return (derror("/!\\ POST not allowed"), setup->setCode(405), 405);
	if (request->getMethod() == "DELETE" && !(request->getLocation()->getPermission() & DEL_PERM))
		return (derror("/!\\ DELETE not allowed"), setup->setCode(405), 405);

	std::cerr << "extensionName: " << setup->getExtensionName() << std::endl;
	if (request->getLocation()->getCgiPerm().find(setup->getExtensionName()) != request->getLocation()->getCgiPerm().end())
		std::cerr << request->getLocation()->getCgiPerm().find(setup->getExtensionName())->second << std::endl;
	if (request->getLocation()->getCgiPerm().find(setup->getExtensionName()) != request->getLocation()->getCgiPerm().end())
		return (this->cgiMode(request, setup, client_fd));

	if (request->getMethod() == "POST")
		return (this->postMode(request, setup, client_fd));
	if (request->getMethod() == "DELETE")
		return (this->deleteMode(request, setup, client_fd));
	if (request->getMethod() == "GET")
		return (this->getMode(request, setup, client_fd));
	return (derror("/!\\ Not Implemented"), setup->setCode(501), 501);
}
