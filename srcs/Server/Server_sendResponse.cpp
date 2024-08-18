/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_sendResponse.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/28 10:07:30 by lgiband           #+#    #+#             */
/*   Updated: 2022/12/02 15:08:14 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <algorithm>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "WebServer.hpp"
#include "Cache.hpp"
#include "utils.hpp"

extern int flags;

int	WebServer::removeResponse(int client_fd)
{
	Cache *cache = 0;

	for (std::vector<Response>::iterator it = this->_all_response.begin(); it != this->_all_response.end(); it++)
	{
		if (it->getFd() == client_fd)
		{
			if (it->getFilename() != "")
				cache = this->getCache(it->getFilename());
			if (cache)
				cache->setUsers(cache->getUsers() - 1);
			if (flags & FLAG_VERBOSE)
				std::cerr << "[ Client disconnected on " << client_fd << " ]" << std::endl;
			close(client_fd);
			this->_all_response.erase(it);
			return (1);
		}
	}
	return (0);
}

int	WebServer::sendHeader(int client_fd, Response *response)
{
	int						sended;

	sended = send(client_fd, response->getHeader().c_str(), (int)response->getHeader().size(), MSG_NOSIGNAL | MSG_DONTWAIT | MSG_MORE);
	if (sended == -1)
	{
		perror("Send failed");
		this->removeResponse(client_fd);
	}
	else if (sended != (int)response->getHeader().size())
		response->eraseHeader(0, sended);
	else if (response->getBodySize() == 0)
	{
		sended = send(client_fd, "", 1, MSG_NOSIGNAL | MSG_DONTWAIT);
		this->removeResponse(client_fd);
	}
	else if (response->getBodySize() > 0)
		response->setStatus(1);
	return (0);
}

int	WebServer::sendBody(int client_fd, Response *response)
{
	int	sended;

	sended = send(client_fd, response->getBody().c_str(), std::min((int)response->getBody().size(), SEND_SIZE), MSG_NOSIGNAL | MSG_DONTWAIT);
	if (sended == -1)
	{
		perror("Send failed");
		this->removeResponse(client_fd);
	}
	else if (sended == (int)response->getBody().size())
		this->removeResponse(client_fd);
	else
		response->eraseBody(0, sended);
	return (0);
}

int	WebServer::sendFile(int client_fd, Response *response)
{
	Cache	*cache;
	int		readed;
	int		sended;

	cache = this->getCache(response->getFilename());
	if (!cache)
		this->removeResponse(client_fd);

	cache->getStream()->seekg(response->getPosition());
	readed = cache->getStream()->readsome(this->_send_buffer, std::min(SEND_SIZE, (int)(response->getBodySize() - response->getPosition())));
	this->_send_buffer[readed] = '\0';
	sended = send(client_fd, this->_send_buffer, readed, MSG_NOSIGNAL | MSG_DONTWAIT);
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Send " << sended << " bytes on " << client_fd << " ]" << " -> " << response->getBodySize() - response->getPosition() - sended <<  std::endl;
	if (sended == -1)
	{
		perror("Send failed");
		this->removeResponse(client_fd);
	}
	else if (sended == (int)response->getBody().size())
		this->removeResponse(client_fd);
	else
		response->setPosition(response->getPosition() + sended);
	return (0);
}
