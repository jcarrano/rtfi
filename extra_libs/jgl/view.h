/*
 *      view.h
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

#ifndef _VIEW_H_
#define _VIEW_H_

#include <SDL/SDL.h>
#include <libjc/common.h>
#include <libjc/cmdopt/optparse.h>

struct view {
	int w, h;
	int fullscreen;
	int mode;
	int showmodes;
	int modes_shown;
	int flags;
	SDL_Surface *screen;
};

/* Nombres de las opciones */

enum {VIEW_SHORT, VIEW_NOSHORT};

#define VIEW_W_SHRT 'W'
#define VIEW_W_LNG "width"

#define VIEW_H_SHRT 'H'
#define VIEW_H_LNG "height"

#define VIEW_FS_SHRT 'F'
#define VIEW_FS_LNG "fullscreen"

#define VIEW_PMODES_SHRT TERM
#define VIEW_PMODES_LNG "get-modes"

#define VIEW_MODE_SHRT	TERM
#define VIEW_MODE_LNG "mode"

enum { VIEW_W, VIEW_H, VIEW_FS, VIEW_PMODES, VIEW_MODE, VIEW_N_REGLAS};

/* Funciones */

/* NOTA: Es necesario hacer SDL_Init(SDL_INIT_VIDEO) y SDL_Quit(), ya que estas
 * 	funciones no lo hacen.
 */

struct view view_defaults(int width, int height, int flags);
	/* Inicializa la estructura y establece valores por defecto */

extern void view_set_parse(struct opt_rule reglas[VIEW_N_REGLAS],
					struct view *vport, int noshort);
	/* Inserta las opciones para configurar la pantalla.
	 * noshort = 1 indica que no se deben incluir las opciones cortas
	 * 		(de una sola letra). Esto es util cuando se precisan
	 * 		esas letras para otra cosa
	 * reglas[] debe tener al menos VIEW_N_REGLAS espacios disponibles
	 */

extern int view_postparse(struct view *vport);
	/* Llámese luego de generic_parser, para validar las opciones e imprimir
	 * los modos de video si es necesario
	 * Devielve -E_OK, -E_BADCFG, -E_DONEHELP (cuando imprime los modos de 
	 * 	video), etc...
	 */

extern int mkview(struct view *vport);
	/* Crea la superficie de SDL de acuerdo a la config.
	 * Devielve -E_OK, -E_BADCFG, -E_DONEHELP (cuando imprime los modos de 
	 * 	video), etc...
	 */ 

extern SDL_Surface *view_screen(const struct view *vport);

#endif /* _VIEW_H_ */
