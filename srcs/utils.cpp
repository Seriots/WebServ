/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmauguin <fmauguin@student.42.fr >         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/30 16:34:10 by lgiband           #+#    #+#             */
/*   Updated: 2022/12/05 12:43:55 by fmauguin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <algorithm>
#include <cstring>
#include <stack>
#include <string>
#include <cstdlib>

#include <sys/stat.h>

std::string uriDecode(const std::string &src)
{
	std::string result;
	int			i = 0;

	while (src[i])
	{
		if (src[i] == '%')
		{
			if (src[i + 1] && src[i + 2] && std::isxdigit(src[i + 1]) && std::isxdigit(src[i + 2]))
			{
				result += (char)std::strtol(src.substr(i + 1, 2).c_str(), NULL, 16);
				i += 2;
			}
		}
		else if (src[i] == '+')
			result += ' ';
		else
			result += src[i];
		i++;
	}
	return (result);
}

std::string	reformatUri(std::string const& uri)
{
	std::string				new_uri;
	std::string::size_type	start = 0;
	std::string::size_type	end = 0;
	std::stack<std::string>	stack;
	std::string				tmp;

	if (uri.find("/") == std::string::npos)
	{
		new_uri = uri;
		return (new_uri);
	}
	while (end != uri.size())
	{
		if (start != 0)
			start = uri.find('/', end);
		while (uri.find('/', start + 1) == 0)
			start++;
		if (uri[start] == '/')
			start++;
		end = uri.find('/', start);
		if (end == std::string::npos)
			end = uri.size();
		tmp = uri.substr(start, end - start);
		if (tmp == "..")
		{
			if (!stack.empty())
				stack.pop();
		}
		else if (tmp != "." && tmp != "")
			stack.push(tmp);
		start = end;
	}
	while (!stack.empty())
	{
		new_uri = stack.top() + "/" + new_uri;
		stack.pop();
	}
	if (uri[0] == '/')
		new_uri = "/" + new_uri;
	if (uri[uri.size() - 1] == '/' && new_uri[new_uri.size() - 1] != '/')
		new_uri = new_uri + "/";
	else if (uri[uri.size() - 1] != '/' && new_uri[new_uri.size() - 1] == '/')
		new_uri = new_uri.erase(new_uri.size() - 1);
	if (new_uri.size() == 0)
		new_uri = "/";
	return (new_uri);
}

bool	doesPathExist(std::string const& path)
{
	struct stat		buf;

	if (stat(path.c_str(), &buf) == -1)
		return (false);
	return (true);
}

bool	isPathReadable(std::string const& path)
{
	struct stat		buf;

	if (stat(path.c_str(), &buf) == -1)
		return (false);
	if (buf.st_mode & S_IRUSR)
		return (true);
	return (false);
}

bool	isPathWriteable(std::string const& path)
{
	struct stat		buf;

	if (stat(path.c_str(), &buf) == -1)
		return (false);
	if (buf.st_mode & S_IWUSR)
		return (true);
	return (false);
}

bool	isDirectory(std::string const& path)
{
	struct stat		buf;

	if (stat(path.c_str(), &buf) == -1)
		return (false);
	if (S_ISDIR(buf.st_mode))
		return (true);
	return (false);
}

bool	isFile(std::string const& path)
{
	struct stat		buf;

	if (stat(path.c_str(), &buf) == -1)
		return (false);
	if (S_ISREG(buf.st_mode))
		return (true);
	return (false);
}

std::string	generateRandomCookie(int size, std::string base)
{
	std::string	cookie;
	int			i = -1;

	cookie.reserve(size + 2);
	while (++i <= size)
		cookie += base[rand() % base.size()];
	return (cookie);
}

int myToUpper(int c)
{
	if (c >= 'a' && c <= 'z')
		return (c - 32);
	if (c == '-')
		return ('_');
	return (c);
}
