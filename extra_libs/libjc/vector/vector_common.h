/*
 *      vector_common.c
 *      
 *      Copyright 2011 Juan I Carrano <juan@carrano.com.ar>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#ifndef _VECTOR_COMMON_H_
#define _VECTOR_COMMON_H_

struct p_vector {
	float value, titha;
};

struct r_vector {
	float x, y;
};

struct limit {
	float min, max;
};

const struct r_vector r_zero = {0, 0};

#endif /* _VECTOR_COMMON_H_ */


