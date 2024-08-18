/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_init.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/23 11:24:38 by lgiband           #+#    #+#             */
/*   Updated: 2022/12/04 14:08:18 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <map>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

#include "WebServer.hpp"
#include "utils.hpp"

extern int flags;

int	WebServer::addDuoCS(int client, int server)
{
	if (this->_duoCS.find(client) != this->_duoCS.end())
		this->_duoCS.erase(client);
	this->_duoCS.insert(std::pair<int, int>(client, server));
	return (0);
}

int WebServer::addClientIP(int client, std::string const &ip)
{
	if (this->_clientIP.find(client) != this->_clientIP.end())
		this->_clientIP.erase(client);
	this->_clientIP.insert(std::pair<int, std::string>(client, ip));
	return (0);
}

int WebServer::create_socket(std::string port, std::string ip)
{
	int server_fd;
	struct sockaddr_in address;
	int on = 1;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
		return (perror("/!\\ Socket creation failed /!\\"), -1);

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &on, sizeof(int)) == -1)
		return (close(server_fd), perror("/!\\ Setsockopt failed /!\\"), -1);

	if (fcntl(server_fd, F_SETFL, O_NONBLOCK) == -1)
		return (close(server_fd), perror("/!\\ Fcntl failed /!\\"), -1);

	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(ip.c_str());
	address.sin_port = htons(std::atoi(port.c_str()));
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1)
		return (close(server_fd), perror("/!\\ Bind failed /!\\"), -1);
	
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ Bind " << ip << ":" << port << " on " << server_fd << " ]" << std::endl;
	
	if (listen(server_fd, MAX_LISTEN) == -1)
		return (close(server_fd), perror("/!\\ Listen failed /!\\"), -1);

	return (server_fd);
}

int WebServer::init(void)
{
	int max = this->_virtual_servers.size();
	struct epoll_event event;

	std::cout << "\n=====================INIT====================\n" << std::endl;

	std::memset(&event, 0, sizeof(event));
	this->_epoll_fd = epoll_create1(0);

	/*Create all the sockets corresponding to servers in the config file*/
	for (std::vector<VirtualServer>::iterator it = this->_virtual_servers.begin(); it != this->_virtual_servers.end(); it++)
	{
		if (this->isNewInterface(it->getHost() + ":" + it->getPort()))
		{
			it->setFd(this->create_socket(it->getPort(), it->getHost()));
			if (it->getFd() == -1)
			{
				this->_virtual_servers.erase(it);
				it--;
			}
			else
			{
				this->_duoSI.insert(std::pair<int, std::string>(it->getFd(), it->getHost() + ":" + it->getPort()));
				event.data.fd = it->getFd();
				event.events = EPOLLIN;
				epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, it->getFd(), &event);
			}
		}
	}
	if (flags & FLAG_VERBOSE)
		std::cerr << "[ " << this->getVirtualServers().size() << "/" << max << " servers created" << " ]" << std::endl;
	if (this->getVirtualServers().size() == 0)
		return (derror("/!\\ No server created /!\\"), 1);
	return (0);
}
