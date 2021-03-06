/*
 * \brief  List for firmware images and their sizes
 * \author Josef Soentgen
 * \date   2014-03-26
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _FIRMWARE_LIST_H_
#define _FIRMWARE_LIST_H_

typedef __SIZE_TYPE__ size_t;

struct Firmware_list
{
	char const *name;
	size_t      size;
};

#endif /* _FIRMWARE_LIST_H_ */

