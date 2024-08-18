/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_cgi.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/01 15:18:26 by fmauguin          #+#    #+#             */
/*   Updated: 2022/12/05 13:28:48 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <algorithm>
#include <iostream>

#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <cstdlib>

#include "WebServer.hpp"
#include "utils.hpp"

extern int flags;

bool	WebServer::isCgi(int file_fd)
{
	if (this->_cgiFD.find(file_fd) != this->_cgiFD.end())
		return (true);
	return (false);
}

bool	WebServer::isCgiClient(int client_fd)
{
	for (std::map<int, int>::iterator it = this->_cgiFD.begin(); it != this->_cgiFD.end(); it++)
	{
		if (it->second == client_fd)
		{
			this->_file_fd = it->first;
			return (true);
		}
	}
	return (false);
}

int	WebServer::closeCgiResponse(int client_fd, int file_fd)
{
	epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
	epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, file_fd, NULL);
	this->removeResponse(client_fd);
	this->_cgiFD.erase(file_fd);
	close(file_fd);
	return (0);
}

int	WebServer::cgiSendResponse(int client_fd)
{
	int			file_fd;
	int			sended;
	Response	*response;

	//if (flags & FLAG_VERBOSE)
	//	std::cerr << "[ CGI send Response ]" << std::endl;

	file_fd = this->_file_fd;
	response = this->getResponse(client_fd);
	if (!response)
		return (this->closeCgiResponse(client_fd, file_fd));

	if (response->getStatus() != 0 && response->getHeader() != "")
	{
		if (response->getHeader().find("Content-Length: ") != std::string::npos)
			response->setBodySize(std::strtol(response->getHeader().substr(response->getHeader().find("Content-Length: ") + 16).c_str(), NULL, 10));
	}
	if (response->getStatus() != 0)
	{
		if (response->getHeader() != "")
		{
			//std::cout << "CGI response header: " << response->getHeader() << std::endl;
			std::cerr << "CGI response header: " << response->getHeader().substr(0, 50) << std::endl;
			sended = send(client_fd, response->getHeader().c_str(), std::min((size_t)SEND_SIZE, response->getHeader().size()), MSG_NOSIGNAL | MSG_MORE);
			if (sended == -1)
				return (this->closeCgiResponse(client_fd, file_fd));
			std::cerr << "Response Header Size : " << response->getHeader().size() << " -> " << response->getHeader().size() - sended << std::endl;
			response->eraseHeader(0, sended);
		}
		else if (response->getBody() != "")
		{
			sended = send(client_fd, response->getBody().c_str(), std::min((size_t)SEND_SIZE, response->getBody().size()), MSG_NOSIGNAL);
			if (sended == -1)
				return (this->closeCgiResponse(client_fd, file_fd));
			std::cerr << "sended body: " << sended << std::endl;
			response->eraseBody(0, sended);
			response->setPosition(response->getPosition() + sended);
			if (response->getBodySize() != 0 && response->getPosition() >= response->getBodySize())
				return (this->closeCgiResponse(client_fd, file_fd));
			if (response->getBody() == "" && response->getStatus() == 2)
				return (this->closeCgiResponse(client_fd, file_fd));
		}
		else if (response->getStatus() == 2 || response->getBodySize() == (std::string::size_type)-1)
			return (this->closeCgiResponse(client_fd, file_fd));
	}
	return (0);
}

int	WebServer::cgiSetResponse(int file_fd)
{
	int	client_fd;
	int	readed;
	Response *response;
	std::string	buf;
	std::string::size_type	end;

	if (flags & FLAG_VERBOSE)
		std::cerr << "[ CGI set Response ]" << std::endl;

	client_fd = this->_cgiFD.find(file_fd)->second;
	response = this->getResponse(client_fd);
	if (!response)
		return (this->closeCgiResponse(client_fd, file_fd));

	if (response->getStatus() == 0)
	{
		readed = read(file_fd, this->_buffer, BUFFER_SIZE);
		std::cerr << "readed heqder: " << readed << std::endl;
		this->_buffer[readed] = '\0';
		if (readed == 0 || readed == -1)
			return (this->closeCgiResponse(client_fd, file_fd));
		buf = std::string(this->_buffer, readed);
		if ((end = buf.find("\n\n") != std::string::npos) || (end = buf.find("\r\n\r\n") != std::string::npos))
		{
			response->addHeader(buf.substr(0, end + 2));
			response->setBodyAlone(buf.substr(end + 2));
			response->setStatus(1);
		}
		else
			response->addHeader(buf);
	}
	else
	{
		readed = read(file_fd, this->_buffer, BUFFER_SIZE);
		if (readed == -1)
			return (this->closeCgiResponse(client_fd, file_fd));
		std::cerr << "readed body: " << readed << std::endl;
		
		if (readed == 0)
		{
			epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, file_fd, NULL);
			return (response->setStatus(2), 0);
		}
		response->addBodyAlone(std::string(this->_buffer, readed));
	}
	return (0);
}
