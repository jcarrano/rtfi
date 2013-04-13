/*
 *      extra.c
 *      
 *      Copyright 2010 Juan I Carrano <juan@superfreak.com.ar>
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
#include <libjc/cmdopt/extra.h>

#define TERM '\0'

static inline int valid_vtype(t)
{
	return (t == VPARSER_FLOAT) || (t == VPARSER_DOUBLE) ||
				(t == VPARSER_INT) || (t == VPARSER_CHAR);
}

static int filler_strtot(struct vector_data *data, int i, char *str, 
								char **endptr)
{
	char c, *_endptr = str;
	union {
		float fl;
		double dbl;
		int n;
	} d;
	int ins, ret;
	
	if (i >= data->n_elem) {
		ins = 0;
		if (!data->ignore_overfl) {
			ret = -PARSE_BADSYNTAX;
			goto filler_strtot_end;
		}
	} else {
		ins = 1;
	}
	
	switch (data->type) {
	case VPARSER_FLOAT:
		d.fl = strtof(str, &_endptr);
		if (ins)
			data->arr.float_arr[i] = d.fl;
		break;
	case VPARSER_DOUBLE:
		d.dbl = strtod(str, &_endptr);
		if (ins)
			data->arr.double_arr[i] = d.dbl;
		break;
	default:
	case VPARSER_INT:
		d.dbl = strtol(str, &_endptr, 0);
		if (ins)
			data->arr.int_arr[i] = d.dbl;
		break;
	case VPARSER_CHAR:
		if((c = str[0]) != TERM)
			_endptr = str + 1;
		if (ins)
			data->arr.char_arr[i] = c;
		break;
	}
	
	if (*str == TERM) {
		ret = 1;
	} else if (str == _endptr) {
		ret = -PARSE_BADSYNTAX;
	} else {
		ret = -PARSE_OK;
	}
	if (endptr != NULL)
		*endptr = _endptr;
	
filler_strtot_end:
	return ret;
}

static int vect_continue(struct vector_data *data, char *value, char **endptr)
{
	int ret = -PARSE_OK, i;
	
	if(*value != TERM)
	{
		for (i = 0; value[i] == data->sep[i] && value[i] != TERM; i++);
		if (data->sep[i] != TERM)
			ret = -PARSE_BADSYNTAX;
		if (endptr != NULL)
			*endptr = &value[i];
	} else {
		ret = 1;
	}
	
	return ret;
}

int vector_parser(char *key, char *value, void *_data, const char **_msg)
{
	struct vector_data *data = (struct vector_data*)_data;
	int ret = -PARSE_OK, i = 0, vc = -PARSE_OK, fc = -PARSE_OK;
	char *end, *msg = NULL;
	
	if (value != NULL && data!=NULL && valid_vtype(data->type)) {
		do {
			vc = filler_strtot(data, i++, value, &end);
			fc = vc;
			if (vc)
				break;
			value = end;
			vc = vect_continue(data, value, &end);
			if (vc)
				break;
			value = end;
		} while (1);
		
		if (vc > 0 && i < data->n_elem && !data->ignore_underfl)
			vc = -PARSE_BADSYNTAX;
		
		if (vc < 0) {
			msg = data->msg;
			ret = vc;
		}
		
		if (data->read != NULL) {
			if (fc < 0)
				i--;
			*data->read = i > data->n_elem ? data->n_elem : i;
		}
	} else {
		ret = -PARSE_BADCONFIG;
	}
	
	if(_msg != NULL )
		*_msg = msg;
	
	return ret;
}

struct vector_data vector_parser_conf(size_t n_elem, char *sep, int type,
			int ignore_overfl, int ignore_underfl, char *msg,
						void *array, int *read)
{
	struct vector_data ret;
	
	ret.type = type;
	switch (type) {
	default:
	case VPARSER_FLOAT:
		ret.arr.float_arr = (float*)array;
		break;
	case VPARSER_DOUBLE:
		ret.arr.double_arr = (double*)array;
		break;
	case VPARSER_INT:
		ret.arr.int_arr = (int*)array;
		break;
	case VPARSER_CHAR:
		ret.arr.char_arr = (char*)array;
		break;
	}
	
	ret.n_elem = n_elem;
	ret.sep = sep;
	ret.ignore_overfl = ignore_overfl;
	ret.ignore_underfl = ignore_underfl;
	ret.msg = msg;
	ret.read = read;
	
	return ret;
}

extern void set_vector_parser(struct opt_rule *regla, struct vector_data *cfg)
{
	set_parse_custom(regla, vector_parser, cfg);
}

/* STRING Collector */

struct str_coll_data str_collector_conf(size_t n_elem, char **store, int strict)
{
	struct str_coll_data r;
	
	r.n_elem = n_elem;
	r.store = store;
	r.strict = strict;
	r.n = 0;
	
	return r;
}

int str_collector(int dummy, char *str, void *_cfg)
{
	struct str_coll_data *cfg = (struct str_coll_data*)_cfg;
	int ret = PARSE_OK;
	
	if (cfg->n < cfg->n_elem) {
		cfg->store[cfg->n] = str;
		cfg->n++;
	} else if (cfg->strict) {
		ret = PARSE_BADSYNTAX;
	}
	
	return ret;
}

int str_coll_n_read(struct str_coll_data *cd)
{
	return cd->n;
}
