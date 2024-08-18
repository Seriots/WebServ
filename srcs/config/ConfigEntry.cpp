// ************************************************************************** //
//                                                                            //
//                                                        :::      ::::::::   //
//   ConfigEntry.cpp                                    :+:      :+:    :+:   //
//                                                    +:+ +:+         +:+     //
//   By: gtoubol <marvin@42.fr>                     +#+  +:+       +#+        //
//                                                +#+#+#+#+#+   +#+           //
//   Created: 2022/11/17 10:32:53 by gtoubol           #+#    #+#             //
//   Updated: 2022/11/17 12:29:45 by gtoubol          ###   ########.fr       //
//                                                                            //
// ************************************************************************** //
#include <cctype>
#include <string>
#include "ConfigEntry.hpp"

ConfigEntry::ConfigEntry(std::string const& line, size_t ln): level(0), key(""), value(""), hasdelimiter(false), line_nb(ln)
{
	std::string::const_iterator it;
	std::string::const_reverse_iterator rit;

	for (it = line.begin(); it != line.end(); ++it)
	{
		if (this->isLevelChar(*it))
			++level;
		else
			break;
	}
	for (; it != line.end(); ++it)
	{
		if (this->isKeyChar(*it))
			key.push_back(*it);
		else
			break;
	}
	// test the delimiter presence ////////////////////////////////////////////
	if (it != line.end())
	{
		if (this->isDelimiter(*it))
		{
			++it;
			hasdelimiter = true;
		}
	}
	// strip the value ////////////////////////////////////////////////////////
	for (; it != line.end(); ++it)
	{
		if (not isspace(*it))
			break ;
	}
	for (rit = line.rbegin(); rit.base() != it; ++rit)
	{
		if (not isspace(*rit))
			break ;
	}
	value.assign(it, rit.base());
}

size_t	ConfigEntry::getLevel(void) const
{
	return (this->level);
}

std::string const& ConfigEntry::getKey(void) const
{
	return (this->key);
}

std::string const& ConfigEntry::getValue(void) const
{
	return (this->value);
}

bool	ConfigEntry::isLevelChar(const char c) const
{
	return (c == ' ');
}

bool	ConfigEntry::isKeyChar(const char c) const
{
	if (isalnum(c) || c == '_' || c == '-')
		return (true);
	return (false);
}

bool	ConfigEntry::isDelimiter(const char c) const
{
	return (c == ':');
}

bool	ConfigEntry::isValueEmpty(void) const
{
	for (std::string::const_iterator it = this->value.begin();
		 it != this->value.end();
		++it)
	{
		if (!isspace(*it))
			return (false);
	}
	return (true);
}

bool	ConfigEntry::hasDelimiter(void) const
{
	return (this->hasdelimiter);
}

size_t	ConfigEntry::getLineNumber() const
{
	return (this->line_nb);
}
