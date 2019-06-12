/*
 *      view.c
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

#include "view.h"

#define NO_MODE ((SDL_Rect**)(0))
#define ALL_MODES ((SDL_Rect**)(-1))

#define VIEW_MODE_NA (-1)
#define VIEW_MODE_UNSET (-1)
#define TERM '\0'

static int meta_modes(int flags, int print, int n ,SDL_Rect **m)
{
	SDL_Rect **modes;
	int i = VIEW_MODE_NA;

	modes = SDL_ListModes(NULL, flags);

	if (modes == NO_MODE)
		printf("No modes\n");
	else if (modes == ALL_MODES)
		printf("All modes\n");
	else
		for(i = 0; modes[i]; i++)
			if (print)
				printf("[%d] %d x %d\n", i, modes[i]->w,
								modes[i]->h);

	if (m != NULL) {
		if (n < i && n >= 0)
			*m = modes[n];
		else
			*m = NULL;
	}

	return i;
}

static int maxmode(int flags)
{
	return meta_modes(flags, 0, 0, NULL);
}

static void view_print_modes(int flags)
{
	meta_modes(flags, 1, 0, NULL);
}

static SDL_Rect *get_mode(int flags, int mode)
{
	SDL_Rect *ret;

	meta_modes(flags, 0, mode, &ret);

	return ret;
}

static int can_fullscreen(int flags)
{
	return SDL_ListModes(NULL, flags) != NO_MODE;
}

static int can_freescreen(int flags)
{
	return SDL_ListModes(NULL, flags) == ALL_MODES;
}

/* Funciones públicas */

struct view view_defaults(int width, int height, int flags)
{
	struct view vport;

	vport.w = width;
	vport.h = height;
	vport.fullscreen = 0;
	vport.flags = SDL_HWSURFACE | flags;
	vport.mode = VIEW_MODE_UNSET;
	vport.showmodes = 0;
	vport.modes_shown = 0;
	vport.screen = NULL;

	return vport;
}

void view_set_parse(struct opt_rule reglas[VIEW_N_REGLAS], struct view *vport,
					int noshort)
{
#define SRT(c) ((noshort == VIEW_NOSHORT)? TERM : (c))
	set_parse_int(&reglas[VIEW_W], &vport->w);
	set_parse_meta(&reglas[VIEW_W], SRT(VIEW_W_SHRT),
				VIEW_W_LNG, "Ancho de la ventana");

	set_parse_int(&reglas[VIEW_H], &vport->h);
	set_parse_meta(&reglas[VIEW_H], SRT(VIEW_H_SHRT),
				VIEW_H_LNG, "Altura de la ventana");

	set_parse_bool(&reglas[VIEW_FS], &vport->fullscreen);
	set_parse_meta(&reglas[VIEW_FS], SRT(VIEW_FS_SHRT), VIEW_FS_LNG,
		"Vista de pantalla completa. Si tiene mas de un monitor, \
se recomienda elegir un modo");

	set_parse_int(&reglas[VIEW_MODE], &vport->mode);
	set_parse_meta(&reglas[VIEW_MODE], SRT(VIEW_MODE_SHRT), VIEW_MODE_LNG,
		"Usar # de resolucion. Implica "VIEW_FS_LNG". Si no se especifica\
se usara la mayor resolucion posible");

	set_parse_bool(&reglas[VIEW_PMODES], &vport->showmodes);
	set_parse_meta(&reglas[VIEW_PMODES], SRT(VIEW_PMODES_SHRT),
		VIEW_PMODES_LNG, "Mostrar las resoluciones disponibles");
}

int view_postparse(struct view *vport)
{
	int r;

	if (vport->mode != VIEW_MODE_UNSET
	   && !(vport->mode >= 0 || vport->mode < maxmode(vport->flags))) {
		r = -E_BADCFG;
	} else {
		 r = -E_OK;
	}
	if (vport->showmodes) {
		if (r == -E_OK)
			r = -E_DONEHELP;
		if (!vport->modes_shown) {
			view_print_modes(vport->flags|SDL_FULLSCREEN);
			vport->modes_shown = 1;
		}
	}


	return r;
}

int mkview(struct view *vport)
{
	int ret;

	if ((ret = view_postparse(vport) == -E_OK) || ret == -E_DONEHELP) {
		ret = -E_OK;

		if (vport->mode != VIEW_MODE_UNSET)
			vport->fullscreen = 1;

		if (vport->fullscreen) {
			if (can_fullscreen((vport->flags |= SDL_FULLSCREEN))) {
				if (!can_freescreen(vport->flags)) {
					SDL_Rect *m;
					m = get_mode(vport->flags, vport->mode);
					if (m != NULL) {
						vport->h = m->h;
						vport->w = m->w;
					} else {
						vport->h = 0;
						vport->w = 0;
					}
				}
			} else {
				ret = -E_BADCFG;
			}
		}

		if (ret == -E_OK) {
			if ((vport->screen = SDL_SetVideoMode(vport->w,
			     vport->h, 0, vport->flags)) == NULL)
				ret = -E_GRAPHIC;
		}
	}

	return ret;
}

SDL_Surface *view_screen(const struct view *vport)
{
	return vport->screen;
}
