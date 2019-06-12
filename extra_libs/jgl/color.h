/*
 *      color.h
 *
 *      Copyright 2011 Juan I Carrano <juan@carrano.com.ar>
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
#ifndef _COLOR_H_
#define _COLOR_H_

struct RGB {
	unsigned char r, g, b;
};

struct HCV {
	float h, c, v;
};

#include "color_inlines.h"

extern float rgb_hue(struct RGB col);
	/* hue between (-pi, pi) */

extern float rgb_val(struct RGB col);
	/* value of the components of projection of (r, g, b) into (1, 1, 1) */

extern float rgb_crom(struct RGB col);
	/* distance from (r, g, b) to line u*(1, 1, 1) */

extern struct RGB rgb_compo(float hue, float crom, float value);
	/* warning!!! not well done!! */

extern struct RGB hcv2rgb(struct HCV col);

extern struct HCV rgb2hcv(struct RGB col);

extern void convolve3x3(struct RGB *img, const int W, const int H,
					const int kernel[3][3], const int div);

#endif /* _COLOR_H_ */
