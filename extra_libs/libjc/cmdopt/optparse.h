/*
 *      optparse.h
 *      
 *      Copyright 2010 Juan I Carrano <juan@carrano.com.ar>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 */


#ifndef _PARSE_H_
#define _PARSE_H_

#include <stdlib.h>

/* Este es el idioma de generic_parser y amigos, y los callback deben usarlos
 * para indicar su exito o fracaso */

#define PARSE_NO_SHORT '\0'

#define REGLA_SIZE (sizeof(struct opt_rule))

enum {PARSE_OK, PARSE_NOMEM, PARSE_BADSYNTAX, PARSE_BADCONFIG, PARSE_REQHELP};

enum ACCIONES { 
    PARSE_IGNORE, 		/* ignora el key y el valor que le sigue */
    PARSE_IGNORE_SWITCH,	/* ignora solo el key*/

    CUSTOM_ACTION, 	/*la accion se delega a un callback :
			int callback(char *clave, char *valor, void *data)
			el cback y los datos van en opt_rule.data.d_custom */

    PARSE_INT, 	/*parsea el valor como un entro y lo graba en *data.d_int */ 

    SET_BOOL, 		/* set/unset : no reqieren valor. ponen en 1 o en 0 */
    UNSET_BOOL, 	/* los contenidos de data.d_bool  */

    PARSE_DOUBLE,  	/*idem int, pero con *data.d_double */
    PARSE_FLOAT,  	/*idem int, pero con *data.d_float*/

    PARSE_STR, 		/*graba una string que es una copia literal del valor en
			*data.str. Luego hay que liberarlo con free */

    PARSE_STR_NOCOPY,	/*idem anterior, salvo que no hace una copia, 
			simplemente copia el puntero. Puede ser tan practico como
			peligroso, pero si se esta procesando argv y argc y no
			se van a modificar los strings, mejor que PARSE_STR */

    DO_HELP		/* Salvo que los sgtes sean NULL, los imprime:
			el helpstr, y para cada opt_rule:
			short_id <tab> long_id <tab> descripcion 
			Ademas, causa que el parser aborte con un error*/
};

#define KEYS_NEED_VALUE { PARSE_IGNORE, CUSTOM_ACTION, PARSE_INT, \
	PARSE_DOUBLE, PARSE_FLOAT, PARSE_STR, PARSE_STR_NOCOPY }

struct opt_rule {
	char short_id;	/* estilo -w Puede ser TERM si no hay disponible */
	const char *long_id;	/* estlo --width . Puede ser null*/
	const char *desc; 	/* descripcion, para la ayuda. Puede ser null */
	int accion; 	/* accion a realizar, ver en enum ACCIONES */
	union {
		int *d_int;
		int *d_bool;
		double *d_double;
		float *d_float;
		char **d_str;
		/* prototipo para el callback:
	int callback(char *key, char *value, void *data, const char *msg)  */
		struct {
			void *data;
			int (*callback)(char *, char*, void*, const char**);
		} d_custom;
	} data;
};

struct opt_conf {
	char *helpstr;
	struct opt_rule *reglas;
	int n_reglas; /* Importante, si no, hace un overrun */
	int ignore_argv0; /* hay que saltear argv[0] ? */
	
	/* int arg_parser(int current_index, char *valor, void *userdata)
	 * Se llama cada vez que hay un valor sin clave. El primer argumento
	 * dice que numero de argumento es, util para identificar argv[0] 
	 * Se puede dejar en null: Si ALLOW_NULL_ARGPARSER esta definido,
	 * no causa un error de parseo, si no, simplemente sale con PARSE_ERR*/
	int (*arg_parser)(int, char *, void*);
	void *arg_parser_data;
};

extern int generic_parser(int argc, char *argv[], struct opt_conf config);

/* inicializadores */

extern struct opt_conf new_conf(struct opt_rule *reglas, size_t n_reglas, 
	char *helpstr, int ignore_argv0, int (*arg_parser)(int, char *, void*) 
						, void *arg_parser_data);

#define INIT_PROTO(name, type) extern void name(struct opt_rule *regla, type *data)

extern void set_parse_meta(struct opt_rule *regla, char short_id, 
					const char *long_id, const char *desc);

extern void set_parse_ignore(struct opt_rule *regla);
extern void set_parse_ignore_sw(struct opt_rule *regla);
extern void set_parse_help(struct opt_rule *regla);

extern void set_parse_custom(struct opt_rule *regla,
		int (*callback)(char *, char*, void*, const char**), void* data);

/* Estos son del tipo set_parse_*( struct opt_rule *regla, <type> *data) */ 

INIT_PROTO(set_parse_int, int);
INIT_PROTO(set_parse_bool, int);
INIT_PROTO(set_parse_bool_unset, int);
INIT_PROTO(set_parse_double, double);
INIT_PROTO(set_parse_float, float);
INIT_PROTO(set_parse_str, char*);
INIT_PROTO(set_parse_str_nocopy, char*);

/* Esto es por si se quieren usar compound literals*/
#define _OPTION_ (struct opt_rule)
/* Tips para definir reglas:
 1- hacer un enum con los nombres de las variables:
  enum {ALTO, ANCHO, VERBOSE, ETC, .... , CANT_PARAMETROS }
  CANT_PARAMETROS es una constante adicional que nos va a servir para el paso 2
 2- declarar:
  struct opt_rule reglas[CANT_PARAMETROS];
 3- inicializar: no podemos inicialozar en la definicion porque habria que poner
  punteros que que no de pueden calcular cuando se carga el programa
  La manera estandar seria:
    reglas[ALTO].short_id = 'a';
    reglas[alto].ect = ...;
    ... etc ...
  Alternativamente podemos usar compound literals + tags y hacer:
   reglas[ALTO] = _OPTION_{ .short_id = 'a', .long_id= ....etc ....};
   reglas[ANCHO] = _OPTION_{ .short_id = 'w', .long_id= ....etc ....};
 4- Algo similar con struct opt_conf
*/

#endif /* _PARSE_H_ */
