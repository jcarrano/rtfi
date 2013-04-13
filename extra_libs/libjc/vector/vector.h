/*
 * vector.h
 * 
 * Copyright 2012 Juan I Carrano <juan@superfreak.com.ar>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <stdlib.h>
#include <math.h>
#include "vector_common.h"

#ifndef _VKW

#define _VKW extern

extern float rand_f(struct limit lim);

#else

/* non inlining functions*/
float rand_f(struct limit lim)
{
	return fabsf(((float)random())/RAND_MAX) * (lim.max - lim.min) + lim.min;
}

#endif /* _VKW */

_VKW inline struct r_vector r_make(float x, float y)
{
	struct r_vector v;
	v.x = x;
	v.y = y;
	return v;
}

_VKW inline struct p_vector p_make(float value, float titha)
{
	struct p_vector v;
	v.value = value;
	v.titha = titha;
	return v;
}

_VKW inline float r_abs(struct r_vector v)
{
	return hypotf(v.x, v.y);
}

_VKW inline struct r_vector r_scale(struct r_vector v, float a)
{
	v.x *= a;
	v.y *= a;
	
	return v;
}

_VKW inline struct r_vector r_unit(struct r_vector v)
{
	return r_scale(v, 1.0 / r_abs(v));
}

_VKW inline float pmod(float x, float y)
{
	return x - y * floor(x / y);
}

_VKW inline float r_abs2(struct r_vector v)
{
	return v.x* v.x + v.y*v.y;
}

_VKW inline struct p_vector p_scale(struct p_vector v, float a)
{
	v.value *= a;
	
	return v;
}

_VKW inline struct r_vector r_sum(struct r_vector v1, struct r_vector v2)
{
	v1.x += v2.x;
	v1.y += v2.y;
	
	return v1;
}

_VKW inline struct r_vector r_subs(struct r_vector v1, struct r_vector v2)
{
	v1.x -= v2.x;
	v1.y -= v2.y;
	
	return v1;
}

_VKW inline struct limit artolimit(float a[2])
{
	struct limit r;
	
	r.min = a[0];
	r.max = a[1];
	
	return r;
}

_VKW inline struct r_vector p_to_r(struct p_vector v)
{
	struct r_vector r;
	float s, c;
#ifdef _GNU_SOURCE
	sincosf(v.titha, &s, &c)
#else
	s = sinf(v.titha);
	c = cosf(v.titha);
#endif /* _GNU_SOURCE */

	r.x = v.value * c;
	r.y = v.value * s;
	
	return r;
}

_VKW inline struct limit r_lim(struct r_vector v)
{
	struct limit r;
	r.min = v.x;
	r.max = v.y;
	return r;
}

_VKW inline struct r_vector lim_r(struct limit v)
{
	struct r_vector r;
	r.x = v.min;
	r.y = v.max;
	return r;
}

_VKW inline float clip(float a, struct limit lim)
{
	if (a > lim.max)
		a = lim.max;
	else if (a < lim.min)
		a = lim.min;
	return a;
}

_VKW inline struct r_vector r_clip(struct r_vector v, struct limit lim_x,
							struct limit lim_y)
{
	v.x = clip(v.x, lim_x);
	v.y = clip(v.y, lim_y);
	
	return v;
}

_VKW inline struct r_vector norm_clip(struct r_vector v, float lim)
{
	float vn = r_abs(v);
	
	return (vn < lim)? v: r_scale(v, lim/vn);
}

_VKW inline float absclip(float a, struct limit lim)
{
	if (fabsf(a) > lim.max)
		a = copysignf(lim.max, a);
	else if (fabsf(a) < lim.min)
		a = copysignf(lim.min, a);
	return a;
}

_VKW inline struct r_vector r_absclip(struct r_vector v, struct limit lim_x,
							struct limit lim_y)
{
	v.x = absclip(v.x, lim_x);
	v.y = absclip(v.y, lim_y);
	
	return v;
}

_VKW inline float wrap(float a, struct limit lim)
{
	int range = lim.max - lim.min;
	a = pmod(a - lim.min, range);
	a += lim.min;
	
	return a;
}

_VKW inline struct r_vector r_toruswrap(struct r_vector v, struct limit lim_x,
							struct limit lim_y)
{
	v.x = wrap(v.x, lim_x);
	v.y = wrap(v.y, lim_y);
	
	return v;
}

_VKW inline float r_dist(struct r_vector v1, struct r_vector v2)
{
	return hypotf(v1.x - v2.x, v1.y - v2.y);
}

_VKW inline float r_angle(struct r_vector v)
{
	return atan2(v.y, v.x);
}

_VKW inline float r_dot(struct r_vector v1, struct r_vector v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}

_VKW inline float r_cos2(struct r_vector v1, struct r_vector v2)
{
	return r_dot(v1, v2) / (r_abs(v1) * r_abs(v2) );
}

_VKW inline struct r_vector r_cbrt(struct r_vector v)
{
	v.x = cbrtf(v.x);
	v.y = cbrtf(v.y);
	
	return v;
}

_VKW inline struct r_vector r_normal(struct r_vector v)
{
	struct r_vector r;
	
	r.x = -v.y;
	r.y = v.x;
	return r;
}
/*
_VKW inline struct r_vector r_project(struct r_vector source, struct r_vector dest)
{
	
}
*/

_VKW inline float cross_z(struct r_vector v1, struct r_vector v2)
{
	return v1.x*v2.y - v1.y*v2.x;
}

_VKW inline float cross_z_sign(struct r_vector v1, struct r_vector v2)
{
	return copysignf(cross_z(v1,v2), r_dot(v1, v2));
}

_VKW inline float cross_z_sign_a(struct r_vector v1, struct r_vector v2, float a)
{
	return copysignf(cross_z(v1,v2)+a, r_dot(v1, v2));
}


#endif /* _VECTOR_H_ */
