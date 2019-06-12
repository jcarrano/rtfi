/*
 *      input.c
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

#include <SDL/SDL.h>
#include "input.h"

int QuitFilter(const SDL_Event *event)
{
	if(event->type == SDL_QUIT
	  || ((event->type == SDL_KEYDOWN || event->type == SDL_KEYUP )
			&&  ((event->key.keysym.sym == SDLK_ESCAPE)
			    || (event->key.keysym.sym == SDLK_q)) ) )
		return 1;
	else
		return 0;
}

void flush_events()
{
	SDL_Event dummy;
	while(SDL_PollEvent(&dummy));
}
