// ************************************************************************** //
//                                                                            //
//                                                        :::      ::::::::   //
//   test_configure.cpp                                 :+:      :+:    :+:   //
//                                                    +:+ +:+         +:+     //
//   By: gtoubol <marvin@42.fr>                     +#+  +:+       +#+        //
//                                                +#+#+#+#+#+   +#+           //
//   Created: 2022/11/04 13:16:58 by gtoubol           #+#    #+#             //
//   Updated: 2022/11/25 15:55:25 by gtoubol          ###   ########.fr       //
//                                                                            //
// ************************************************************************** //

#include <exception>
#include "Configure.hpp"


int main(int argc, char **argv) {
	if (argc == 2)
	{
		Configure test(argv[1]);
		if (test.isGood())
			return (0);
		return (1);
	}
	return (0);
}
