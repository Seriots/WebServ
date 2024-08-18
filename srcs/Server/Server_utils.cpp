/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_utils.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/23 13:53:59 by lgiband           #+#    #+#             */
/*   Updated: 2022/12/04 15:13:19 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

#include <sys/epoll.h>
#include <unistd.h>

#include "Request.hpp"
#include "WebServer.hpp"

int WebServer::is_server(int fd)
{
	for (std::vector<VirtualServer>::iterator it = this->_virtual_servers.begin(); it != this->_virtual_servers.end(); it++)
	{
		if (it->getFd() == fd)
			return (fd);
	}
	return (-1);
}

int WebServer::isMe(std::string const &uri, std::string const &path, std::string const &host)
{
	std::string::size_type pos = 0;
	std::string tmp;

	pos = uri.find(host);
	if (pos != std::string::npos)
		pos += host.size();
	else
		pos = 0;
	if (&uri[pos] == path)
		return (true);
	return (false);
}

Request *WebServer::get_fd_request(int fd)
{
	size_t size = this->_all_request.size();

	for (std::vector<Request>::iterator it = this->_all_request.begin(); it != this->_all_request.end(); it++)
	{
		if (it->getFd() == fd)
			return (&(*it));
	}

	Request new_request(fd);
	this->_all_request.push_back(new_request);
	if (size + 1 != this->_all_request.size())
		return (NULL);
	return (&(*(this->_all_request.end() - 1)));
}

void WebServer::remove_fd_request(int fd)
{
	for (std::vector<Request>::iterator it = this->_all_request.begin(); it != this->_all_request.end(); it++)
	{
		if (it->getFd() == fd)
		{
			this->_all_request.erase(it);
			return;
		}
	}
}

void WebServer::clearCache(void)
{
	for (std::vector<Cache>::iterator it = this->_all_cache.begin(); it != this->_all_cache.end(); it++)
	{
		if (it->getUsers() == 0)
		{
			it->getStream()->close();
			delete it->getStream();
			this->_all_cache.erase(it);
			return;
		}
	}
}

void WebServer::clearTimeout(void)
{
	std::map<int, t_pair>::iterator itdel;
	std::map<int, t_pair>::iterator it = this->_timeout.begin();

	if (this->_timeout.empty())
		return;
	while (it != this->_timeout.end())
	{
		if (time(NULL) - it->second.time > REQUEST_TIMEOUT)
		{
			itdel = it;
			it++;
			if (itdel->second.state == 0)
				close(itdel->first);
			else if (itdel->second.state == 1)
			{
				this->remove_fd_request(itdel->first);
				close(itdel->first);
				epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, itdel->first, NULL);
			}
			else if (itdel->second.state == 2)
				this->removeResponse(itdel->first);
			else if (itdel->second.state == 3)
			{
				this->_cgiFD.erase(itdel->first);
				close(itdel->first);
			}
			this->_timeout.erase(itdel);
		}
		else
			it++;
	}
}

bool	WebServer::isNewInterface(std::string const& interface)
{
	for (std::map<int, std::string>::iterator it = this->_duoSI.begin(); it != this->_duoSI.end(); it++)
	{
		if (it->second == interface)
			return (false);
	}
	return (true);
}

std::string	WebServer::getType(std::string const& extension)
{
	if (this->_mimetypes.find(extension) == this->_mimetypes.end())
		return ("text/plain");
	return (this->_mimetypes.find(extension)->second);
}

std::string WebServer::getExtension(std::string const &type)
{
	(void)type;
	return ("");
}

std::string WebServer::getStatus(int code)
{
	if (this->_status_codes.find(code) == this->_status_codes.end())
		return ("Unknown");
	return (this->_status_codes.find(code)->second);
}
