/*
 *      funcs.c
 *      
 *      Copyright 2011 Juan I Carrano <juan@superfreak.com.ar>
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

#include <stdlib.h>
#include <math.h>

inline float fun_r(float t)
{
	return (t > 0)? t : 0;
}

inline float fun_lambda(float t)
{
	return fun_r(1 - fabsf(t));
}

inline float fun_u(float t)
{
	return (t > 0);
}

inline float fun_pi(float t)
{
	return (fabsf(t) < 1/2);
}
