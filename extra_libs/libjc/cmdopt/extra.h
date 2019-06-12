/*
 *      extra.h
 *      
 *      Copyright 2010 Juan I Carrano <juan@carrano.com.ar>
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

#ifndef _PARSERS_EXTRA_H_
#define _PARSERS_EXTRA_H_

#include "optparse.h"

#define ANY_SEP ""

enum { VPARSER_FLOAT, VPARSER_DOUBLE, VPARSER_INT, VPARSER_CHAR};

struct vector_data {
	size_t n_elem;
	char *sep;
	int type;
	int ignore_overfl;
	int ignore_underfl;
	char *msg;
	union {
		double *double_arr;
		float *float_arr;
		int *int_arr;
		char *char_arr;
	} arr;
	int *read;
};

struct str_coll_data {
	size_t n_elem;
	int strict;
	int n;
	char **store;
};

extern struct vector_data vector_parser_conf(size_t n_elem, char *sep, int type,
			int ignore_overfl, int ignore_underfl, char *msg,
						void *array, int *read);

extern void set_vector_parser(struct opt_rule *regla, struct vector_data *cfg);

extern struct str_coll_data str_collector_conf(size_t n_elem, char **store, 
								int strict);

extern void set_str_collector(struct opt_conf, struct str_coll_data *cfg);

#endif /* _PARSERS_EXTRA_H_ */
