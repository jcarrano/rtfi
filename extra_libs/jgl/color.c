/*
 *      color.c
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

#include <math.h>
#define COLOR_EXT
#include "color.h"
#include <libjc/funcs/funcs.h>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif /* M_PI */

#define SQRT_3 1.73205080756887729352744634150587236694280525381
#define MAX_COMPO 255
#define HUE_PERIOD (2*M_PI)

inline float _posit(float a)
{
	return a>0? a: 0;
}

static float _d_3(float a, float b, float c)
{
	return sqrtf(a*a + b*b + c*c);
}

float rgb_hue(struct RGB col)
{	/* hue between (-pi, pi) */
	return atan2f((2.0*col.r - col.g - col.b), SQRT_3*(col.g - col.b));
}

float _rgb_valmod(struct RGB col)
{ /* magnitude of projection of (r, g, b) into (1, 1, 1) */
	return (col.r + col.g + col.b) / SQRT_3;
}

float rgb_val(struct RGB col)
{ /* value of the components of projection of (r, g, b) into (1, 1, 1) */
	return (col.r + col.g + col.b) / 3.0;
}

float rgb_crom(struct RGB col)
{ /* distance from (r, g, b) to line u*(1, 1, 1) */
	float val = rgb_val(col);

	return _d_3(col.r - val, col.g - val, col.b - val);
}

struct RGB rgb_compo(float hue, float crom, float value)
{
	unsigned char c[4];
	struct RGB ret;
	int k;
	const float l1 = HUE_PERIOD/3.0;
	float top = MAX_COMPO*crom;
	float tmp,  crom1 = (1-crom)*MAX_COMPO;
	hue = hue + M_PI;

	for(k=0; k<4; k++){
		tmp = (top*2)*fun_lambda((hue-l1*k)/l1);
		c[k] = (unsigned char)((tmp < top)? tmp: top);
	}
	c[0] += c[3];
	ret.r = (c[0] + crom1)*value;
	ret.g = (c[1] + crom1)*value;
	ret.b = (c[2] + crom1)*value;

	return ret;
}


struct RGB hcv2rgb(struct HCV col)
{
	struct RGB ret = rgb_compo(col.h, col.c, col.v);

	return ret;
}

struct HCV rgb2hcv(struct RGB col)
{
	struct HCV ret;

	ret.h = rgb_hue(col);
	ret.v = rgb_val(col);
	ret.c = rgb_crom(col);

	return ret;
}
