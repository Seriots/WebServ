/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgiband <lgiband@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/23 13:48:17 by lgiband           #+#    #+#             */
/*   Updated: 2022/12/04 15:11:20 by lgiband          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <dirent.h>

#include "Response.hpp"
#include "utils.hpp"

Response::Response() : _fd(-1), _send_status(0), _header(""), _body_size(-1), _position(0), _body(""), _filename("") {}

Response::~Response() {}

void Response::eraseHeader(int start, int end)
{
	this->_header.erase(start, end);
}

void Response::eraseBody(int start, int end)
{
	this->_body.erase(start, end);
}

void Response::addHeader(std::string const &value)
{
	this->_header += value;
}

/*Accesseurs*/
std::string const &Response::getFilename() const
{
	return (this->_filename);
}

std::string const &Response::getHeader() const
{
	return (this->_header);
}

std::string const &Response::getBody() const
{
	return (this->_body);
}

std::string::size_type const &Response::getBodySize() const
{
	return (this->_body_size);
}

std::string::size_type const &Response::getPosition() const
{
	return (this->_position);
}

int const &Response::getFd() const
{
	return (this->_fd);
}

int const &Response::getStatus() const
{
	return (this->_send_status);
}

void Response::setFilename(std::string const &filename)
{
	this->_filename = filename;
}

void Response::setHeader(std::string const &header)
{
	this->_header = header;
}

void Response::setHeader(Setup *setup, std::map<int, std::string> const &status_code, std::multimap<std::string, std::string> const &mime_type, size_t body_size)
{
	std::string type;
	std::string status;

	if (mime_type.find(setup->getExtension()) != mime_type.end() && body_size > 0)
		type = mime_type.find(setup->getExtension())->second;
	else if (body_size > 0)
		type = "text/plain";
	if (status_code.find(setup->getCode()) != status_code.end())
		status = status_code.find(setup->getCode())->second;
	else
		status = "Unknown";

	this->_header = "HTTP/1.1 " + to_string(setup->getCode()) + " " + status + "\r\n";
	this->_header += "Content-Length: " + to_string(body_size) + "\r\n";
	if (body_size > 0)
		this->_header += "Content-Type: " + type + "\r\n";
	this->_header += "Connection: close\r\n";
	this->_header += setup->getFields();
	this->_header += "\r\n";
}

void Response::setBody(std::string const &body)
{
	this->_body = body;
	this->_body_size = this->_body.size();
}

void Response::setBody(int code, std::string const &type)
{
	this->_body = "\
<!DOCTYPE html>\n\
<html>\n\
<head>\n\
<meta charset=\"utf-8\">\n\
<title>Error " + to_string(code) +
				  "</title>\n\
</head>\n\
<body>\n\
	<h1>Error " + to_string(code) +
				  ": " + type + "</h1>\n\
</body>\n\
</html>";
	this->_body_size = this->_body.size();
}

void Response::setBodyAlone(std::string const &body)
{
	this->_body = body;
}

void	Response::addBodyAlone(std::string const& body)
{
	this->_body += body;
}

int	Response::setListingBody(std::string uri, std::string const& root)
{
	DIR *dir;
	struct dirent *ent;
	std::string relative_path;
	std::vector<std::string> files;

	(void)root;
	relative_path = uri;
	relative_path.replace(0, root.size(), "");
	if (relative_path[0] != '/')
		relative_path.insert(0, "/");
	if (uri[0] != '/')
		uri.insert(0, "/");
	if (*(relative_path.end() - 1) != '/')
		relative_path += '/';
	if (*(uri.end() - 1) != '/')
		uri += '/';
	this->_body = "<!DOCTYPE html>\n\
<html>\n<head>\n<title>Index of " +
				  relative_path + "</title>\n</head>\n\
<body>\n<h1 style=\"font-size:30px\">Index of " +
				  relative_path + "</h1>\n<br><br><hr>\n";
	dir = opendir(uri.c_str());
	if (dir == NULL)
		return (500);
	while ((ent = readdir(dir)) != NULL)
		files.push_back(ent->d_name);
	closedir(dir);
	std::sort(files.begin(), files.end());
	for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); it++)
		this->_body += "<a style=\"margin: 5px; font-size: 20px; font-style: italic;\" href=\"" + relative_path + *it + "\">" + *it + "</a><br><hr>\n";
	this->_body += "</body>\n</html>";
	this->_body_size = this->_body.size();
	return (0);
}

void Response::setBodySize(std::string::size_type const &body_size)
{
	this->_body_size = body_size;
}

void Response::setPosition(std::string::size_type const &position)
{
	this->_position = position;
}

void Response::setFd(int const &fd)
{
	this->_fd = fd;
}

void Response::setStatus(int const &status)
{
	this->_send_status = status;
}
