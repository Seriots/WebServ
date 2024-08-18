// ************************************************************************** //
//                                                                            //
//                                                        :::      ::::::::   //
//   ConfigTree.hpp                                     :+:      :+:    :+:   //
//                                                    +:+ +:+         +:+     //
//   By: gtoubol <marvin@42.fr>                     +#+  +:+       +#+        //
//                                                +#+#+#+#+#+   +#+           //
//   Created: 2022/11/23 11:17:36 by gtoubol           #+#    #+#             //
//   Updated: 2022/11/25 18:10:37 by gtoubol          ###   ########.fr       //
//                                                                            //
// ************************************************************************** //

#ifndef CONFIGTREE_H
#define CONFIGTREE_H
#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include "ConfigEntry.hpp"

struct ConfigTree
{
public:
	ConfigTree();
	ConfigTree(ConfigEntry const&);
	virtual ~ConfigTree() {}

	void	print(std::string const&) const;
	std::vector<ConfigTree>& getLeaves();
	std::vector<ConfigTree> const& getLeaves() const;

	std::string const& getKey() const;
	std::string const& getValue() const;
	bool		hasDelimiter() const;
	size_t		getLineNumber() const;

	bool		operator==(std::string const&) const;

private:
	size_t		line;
	std::string	key;
	std::string	value;
	bool		delimiter;
	std::vector<ConfigTree>	leaves;
};

#endif /* CONFIGTREE_H */
