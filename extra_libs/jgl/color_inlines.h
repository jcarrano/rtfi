/*
 *      color_inlines.h
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

#ifndef COLOR_EXT
#undef COLOR_EXT_Q
#define COLOR_EXT_Q
#else
#define COLOR_EXT_Q extern
#endif


COLOR_EXT_Q inline struct RGB rgb(unsigned char r, unsigned char g, unsigned char b)
{
	struct RGB ret;
	ret.r = r;
	ret.g = g;
	ret.b = b;
	return ret;
}

COLOR_EXT_Q inline struct HCV hcv(float h, float c, float v)
{
	struct HCV ret;
	ret.h = h;
	ret.c = c;
	ret.v = v;
	return ret;
}
