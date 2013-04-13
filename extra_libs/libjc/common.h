/*
 *      common.h
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
#ifndef _LIBCOMMON_H_
#define _LIBCOMMON_H_

#define PERROR(msg...) fprintf(stderr, msg)
#define PDUMMY(msg...) {} /* {} avoids problems with non-bracketed control structures */
#define ARSIZE(a) ((sizeof(a))/(sizeof((a)[0])))
#define __MALLOC(S) (S = malloc(sizeof(*(S))))
#define __CALLOC(S) (S = calloc(1, sizeof(*(S))))
#define NMALLOC(S, n) (S = malloc((n)*sizeof(*(S))))
#define NCALLOC(S, n) (S = calloc((n), sizeof(*(S))))

enum {E_OK, E_NOMEM, E_BADCFG, E_GRAPHIC, E_DONEHELP, E_BADARGS, E_OTHER, 
								  N_ECODES}; 

#define SUCCESS (-E_OK)

#endif /* _LIBCOMMON_H_ */
