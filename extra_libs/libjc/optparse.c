/*
 *      optparse.c
 *      
 *      Copyright 2010 Juan I Carrano <juan@superfreak.com.ar>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 */

#include <libjc/cmdopt/optparse.h>
#include <stdio.h>
#include <string.h>

#define TERM '\0'
#define ERROR 0
#define OPT '-'

#define P_ERR(s...) fprintf(stderr, s)
#define STR_ERR(s) fputs(s, stderr)

#define NEED_V_SIZE (sizeof(need_value)/sizeof(int))
#define NEEDS_VALUE(rule) (int_in_arr((rule).accion, need_value, NEED_V_SIZE))

#ifndef DEBUG

#define P_DEBUG(s...) 
#define LOAD_VAR(var, value) if( (var)!=NULL) *(var)=value;

#else /* DEBUG */

#define P_DEBUG P_ERR
#define LOAD_VAR(var, value) if( (var)!=NULL) *(var)=value;\
	else P_DEBUG("Advertencia! puntero nulo:" #var "%c\n", ' ');

#endif /*DEBUG */

#define HELP_STREAM stdout

#define SAFE_FPUTS(str, stream) if( (str)!=NULL) fputs(str, stream);
#define SAFE_FPUTC(c, stream) if( c!=TERM) fputc(c, stream);

#ifndef _cplusplus
enum {false, true};
#endif /* _cplusplus */

/***************************/
static char * isopt(char stri[]);
static int int_in_arr(int x, const int arr[], int n);
static int do_action(struct opt_rule rule, char *key, char *value);
int generic_parser(int argc, char *argv[], struct opt_conf config);
/**********************************/

const int need_value[] = KEYS_NEED_VALUE;

static int int_in_arr(int x, const int arr[], int n)
{	/* Busca un entero x en una lista arr[] de n enteros */
	int ret=0;
	
	if (arr != NULL) {
		while (n>0) {
			if (arr[--n] == x ) {
				ret = 1;
				break;
			}
		}
	}
	
	return ret;
}

static char *isopt(char stri[])
{	/* Si el string es una opcion, devuelve un puntero
	 a su nombre; si no lo es devuelve null */
	char *ret=NULL;
	
	if(stri[0]==OPT)
		ret=&stri[1];
	return ret;
}

static void do_help(const struct opt_conf config)
{	/*no valida */
	int j;

	SAFE_FPUTS(config.helpstr ,HELP_STREAM);
	SAFE_FPUTC('\n', HELP_STREAM);
	for(j=0; j<config.n_reglas; j++){
	    SAFE_FPUTC('-', HELP_STREAM);
	    SAFE_FPUTC(config.reglas[j].short_id, HELP_STREAM);
	    SAFE_FPUTS("\t--", HELP_STREAM);
	    SAFE_FPUTS(config.reglas[j].long_id, HELP_STREAM);
	    SAFE_FPUTC('\t', HELP_STREAM);
	    SAFE_FPUTS(config.reglas[j].desc, HELP_STREAM);
	    SAFE_FPUTC('\n', HELP_STREAM);
	}
}

static int do_action(struct opt_rule rule, char *key, char *value)
{	/*aca asumo que generic_parser me dio todo bien, es decir, la key
	y el value no son null si no deben serlo */
	int ret=PARSE_OK;
	char *nuevo_str=NULL;
	const char *custom_message = NULL;
	
	switch (rule.accion) {
	    case PARSE_IGNORE: case PARSE_IGNORE_SWITCH:
		break;
	    case PARSE_INT:
		LOAD_VAR(rule.data.d_int, atoi(value));
		break;
	    case PARSE_DOUBLE:
		LOAD_VAR(rule.data.d_double, strtod(value, NULL));
		break;
	    case PARSE_FLOAT:
		LOAD_VAR(rule.data.d_float, strtof(value, NULL));
		break;
	    case PARSE_STR_NOCOPY:
		LOAD_VAR(rule.data.d_str, value);
		break;
	    case SET_BOOL:
		LOAD_VAR(rule.data.d_bool, true);
		break;
	    case UNSET_BOOL:
		LOAD_VAR(rule.data.d_bool, false);
		break;
	    case PARSE_STR:
		if(rule.data.d_str != NULL && (nuevo_str=strdup(value))!=NULL){
			*rule.data.d_str = nuevo_str;
		}else {
			P_DEBUG("Fallo PARSE_STR: d_str: %p,", (void*)rule.data.d_str);
			P_DEBUG(" nuevo_str %p\n", nuevo_str);
			if(rule.data.d_str == NULL)
				ret = -PARSE_BADCONFIG;
			else
				ret = -PARSE_NOMEM;
		}
		break;
	    case DO_HELP:
		P_DEBUG("do_action encontro un DO_HELP\n");
		break; /*esta es una meta-accion y tiene que estar implementada
			en generic_parser  */
	    case CUSTOM_ACTION:
		ret = rule.data.d_custom.callback(key, value, 
				rule.data.d_custom.data, &custom_message);
		if (custom_message != NULL){
			STR_ERR(custom_message);
		}
		break;
	    default:
		P_DEBUG("Accion desconocida: %d\n", rule.accion);
		ret = -PARSE_BADCONFIG;
		break;
	}
	
	return ret;
}	

int generic_parser(int argc, char *argv[], struct opt_conf config)
{
	int error=0, i, j, no_more_options = 0;
	char *key, *value, *tmp_key, *pending_opt = NULL;
	struct opt_rule curr_rule;
	
	if (config.ignore_argv0)
		i = 1;
	else
		i = 0;
	
	while( error >= PARSE_OK && i<argc ){
		if ( !no_more_options
		    && ( (pending_opt != NULL) 
		       || ( ((key=isopt(argv[i])) != NULL ) && key[0]!=TERM ))){

			if( pending_opt == NULL && (tmp_key=isopt(key))!=NULL){
				key=tmp_key; /*tmp_key == NULL es nuestro flag 
						de que es una opcion corta */
				if( tmp_key[0] == TERM )
					no_more_options = 1;
			} else if( pending_opt != NULL){
				tmp_key = NULL;
				key = pending_opt;
				pending_opt = NULL; /*reseteo*/
			}
			
			if(!no_more_options) {
				for(j=0; j<config.n_reglas; j++){
					if(   (tmp_key==NULL 
						&& config.reglas[j].short_id == key[0])
					  ||( (tmp_key != NULL) && (config.reglas[j].long_id !=NULL )
					    && strcmp(key, config.reglas[j].long_id)==0)){
						    curr_rule = config.reglas[j];
						    break;
					}
				}
				if (j < config.n_reglas){
					/*ME quedo por ahora <===========> */
					if (curr_rule.accion == DO_HELP){
						/*ME VOY <===========> */
						do_help(config);
						error = -PARSE_REQHELP;
					}else if (NEEDS_VALUE(curr_rule) ){
						if( tmp_key == NULL && key[1]!=TERM){
			/*esto nos permite hacer cosas del tipo -d12.6 */
							value = &key[1];
								
						} else if( (  (tmp_key == NULL 
								&& key[1]==TERM)
							      || tmp_key != NULL
							   ) && i< (argc-1) ) {
					/* i se incrementa SOLO aca y ABAJO */
							/*ME QUEDO <======> */
							value = argv[++i];
						} else {
							/*ME VOY <===========> */
							if(tmp_key == NULL )
								P_ERR("Falta valor para la opcion: %c\n", key[0]);
							else
								P_ERR("Falta valor para la opcion: %s\n", key);
							error = -PARSE_BADSYNTAX;
						}
					
					} else { /*si es un simple switch */
						/*ME QUEDO <======> */
						value = NULL;
						if ( tmp_key == NULL && key[1]!=TERM){
							pending_opt = &key[1];
						}
					}
					if( error >= PARSE_OK )
						 /*ME VOY ???? */
						error =do_action(curr_rule, key, 
									value);
				} else {
					P_ERR("Opcion desconocida: %s\n", key);
					error = -PARSE_BADSYNTAX;
				}
			}
		} else if(config.arg_parser != NULL){
			/*ME VOY ???? */
			error = config.arg_parser(i, argv[i], 
							config.arg_parser_data);
		} else {
			P_ERR("Argumento: %s no tiene clave\n",
					argv[i] );
#ifndef ALLOW_NULL_ARGPARSER
			error = -PARSE_BADSYNTAX; /*ME VOY <===========> */
#endif
		}
		if ( pending_opt == NULL )
			i++; /* i se incrementa SOLO aca y ARRIBA */
	}
	
	return error;
}

/* SETTERS : no le pasen argumentos nulos !!*/

struct opt_conf new_conf(struct opt_rule *reglas, size_t n_reglas, char *helpstr,
			int ignore_argv0, int (*arg_parser)(int, char *, void*) 
						, void *arg_parser_data)
{
	struct opt_conf ret;
	ret.helpstr = helpstr;
	ret.n_reglas = n_reglas;
	ret.reglas = reglas;
	ret.ignore_argv0 = ignore_argv0;
	ret.arg_parser = arg_parser;
	ret.arg_parser_data = arg_parser_data;
	
	return ret;
}

void set_parse_meta(struct opt_rule *regla, char short_id, const char *long_id, 
							const char *desc)
{
	regla->short_id = short_id;
	regla->long_id = long_id;
	regla->desc = desc;
}						

#define SET_ACCION(regla, action) (regla)->accion = (action)
#define MK_SETTER(name, action, type, var) \
	void name(struct opt_rule *regla, type *data) {\
		SET_ACCION(regla, action);\
		regla->data.var = data ;\
	}

void set_parse_ignore(struct opt_rule *regla)
{
	SET_ACCION(regla, PARSE_IGNORE);
}

void set_parse_ignore_sw(struct opt_rule *regla)
{
	SET_ACCION(regla, PARSE_IGNORE_SWITCH);
}

void set_parse_help(struct opt_rule *regla)
{
	SET_ACCION(regla, DO_HELP);
}

void set_parse_custom(struct opt_rule *regla,
		int (*callback)(char *, char*, void*, const char**), void* data)
{
	SET_ACCION(regla, CUSTOM_ACTION);
	regla->data.d_custom.callback = callback;
	regla->data.d_custom.data = data;
}

MK_SETTER(set_parse_int, PARSE_INT, int, d_int)
MK_SETTER(set_parse_double, PARSE_DOUBLE, double, d_double)
MK_SETTER(set_parse_float, PARSE_FLOAT, float, d_float)
MK_SETTER(set_parse_bool, SET_BOOL, int, d_bool)
MK_SETTER(set_parse_bool_unset, UNSET_BOOL, int, d_bool)
MK_SETTER(set_parse_str, PARSE_STR, char*, d_str)
MK_SETTER(set_parse_str_nocopy, PARSE_STR_NOCOPY, char*, d_str)
	
