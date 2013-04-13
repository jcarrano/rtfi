/*
 * images.c
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <semaphore.h>
#include <jgl/view.h>
#include <jgl/color.h>
#include <jgl/input.h>
#include <libjc/common.h>
#include "rtfi.h"
#include "spectral_tables.c"

#ifdef DEBUG
#define PDEBUG PERROR
#else
#define PDEBUG PDUMMY
#endif /*DEBUG*/

#define DEF_WIDTH 800
#define DEF_HEIGHT 600

#define REFLEVEL_PLUS SDLK_UP
#define REFLEVEL_MINUS SDLK_DOWN
#define REFLEVEL_ACCEL KMOD_SHIFT
#define PAUSE SDLK_SPACE
#define NO_ACCEL_AMOUNT 10
#define ACCEL_AMOUNT 100
#define MODE_PLUS SDLK_RIGHT
#define MODE_MINUS SDLK_LEFT

#define TIME_AVG 5
#define FREQ_AVG 5
#define LOWER_THRS (90)
#define PEAK_VALUE (-50) /* ?????????????? */
#define MAX_FPS 60
#define MIN_REFRESH_TIME (1000/MAX_FPS)

#define ICONFILE "tficon.bmp"

enum MODES {ARTFI, AES, PES, SPES, NPES, N_MODES};
const char *modenames[] = {"ARTFI", "AES", "PES", "SPES", "NPES"};

#define DEF_MODE ARTFI

struct ui_ctrl {
	int base_band;
	int paused;
	unsigned int mode;
	int running;
	int quit_requested;
};

static struct ui_ctrl uicontrol = {0, 0, DEF_MODE, 0, 0};

struct start_param {
	int *r;
	void *client;
	sem_t *sem;
};

static Uint32 start_rtfi(Uint32 interval, void *param_);
static int image_run(SDL_Surface *screen);
static int event_parser(void *data);
static int image_prepare(SDL_Surface **screen, int w, int h, int fs);

int main(int argc, char *argv[])
{
	int r = 0, w, h, fs = 0;
	void* client;
	SDL_Surface *screen;
	SDL_Surface *icon;
	SDL_Thread *img_th;
	sem_t sem;
	struct start_param stp;
	
	/* Semaphore init */
	if (sem_init(&sem, 0, 0) != 0) {
		r = errno;
		goto sem_disaster;
	}
	
	/* RTFI initialization */
	client = rtfi_prepare(&r, &sem);
	if (client == NULL) {
		r = -E_OTHER;
		goto rtfi_disaster;
	}
	
	if (argc == 3) {
		w = strtol(argv[1], NULL, 0);
		h = strtol(argv[2], NULL, 0);
	} else {
		w = DEF_WIDTH;
		h = DEF_HEIGHT;
		if(argc == 2 && argv[1][0] == 'f') {
			fs = 1;
		} else if (argc != 1) {
			printf("rtfi visualizer, by Juan I Carrano\n");
			printf("Usage: %s [width height]\n", argv[0]);
			printf("Fullscreen: %s f\n", argv[0]);
			goto not_configured;
		}
	}
	
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
	SDL_WM_SetCaption("RTFI","RTFI");
	
	if ((icon = SDL_LoadBMP(ICONFILE)) != NULL) {
		SDL_WM_SetIcon(icon, NULL);
		SDL_FreeSurface(icon);
	}
	
	if ((r = image_prepare(&screen, w, h, fs)) < 0)
		goto image_disaster;
	
	stp.r = &r;
	stp.client = client;
	stp.sem = &sem;
	
	img_th = SDL_CreateThread(image_run, screen);
	
	SDL_Delay(1000);
	start_rtfi(0, &stp);
	event_parser(NULL);
	/* SDL_Delay(10000); */

	SDL_WaitThread(img_th, NULL);

image_disaster:
	SDL_Quit();
not_configured:
	rtfi_unload(client);
rtfi_disaster:
sem_disaster:
	return -r;
}

static Uint32 start_rtfi(Uint32 interval, void *param_)
{
	struct start_param *param = param_;
	
	*param->r = rtfi_launch(param->client);
	if (*param->r != 0) {
		uicontrol.quit_requested = 1;
		while (uicontrol.running)
			sem_post(param->sem);
	}
	
	return 0;
}

static int image_prepare(SDL_Surface **screen, int w, int h, int fs)
{
	struct view vp;
	int r;
	
	vp = view_defaults(w, h, 0);
	vp.fullscreen = fs;
	r = mkview(&vp);
	if (r < 0)
		*screen = NULL;
	else
		*screen = view_screen(&vp);
	
	return r;
}

static inline void putpixel(SDL_Surface *scr, int i, int j, struct RGB col)
{
	Uint32 Ucol = SDL_MapRGB(scr->format, col.r, col.g, col.b);
	
	((Uint32 *)scr->pixels)[j*scr->pitch/4 + i] = Ucol;
}

static inline float dba(float x)
{
	return 20*log10f(x);
}

static inline float denorm0(float v,  float peak, float ths)
{
	float value = v - peak + ths;
	return (value < ths)? ((value < 0) ? 0 : value) : ths;
}

static inline float denorm1(float v, float peak , float ths)
{
	return denorm0(v, peak, ths)/ths;
}

static inline struct RGB gray(float v)
{
	return rgb(v, v, v);
}

static inline struct RGB iris(float v)
{
	/* v between 0 and 1 */
	/* float v = v; (pasthrough) */
	/*float c = v * (1 - v) * (0.5 - v) * (0.5 - v) * 64;*/
	/*float c = 4*v*(1-v)*((0.5 - v)*(0.5 - v) *11.6569 + 0.5);*/
	float c = 6.75*v*v*v - 13.5 * v*v + 6.75 * v;
	float h = ((v > 0.5)? (v - 1) : v) * 2 * M_PI;
	return hcv2rgb(hcv(h, c, v*v));
}

static inline void pes(float *dst, float *src, int n)
{
	int i;
	
	for (i = n - 1; i >= 0; i--) {
		float acc = src[i];
		int k;
		for (k = 0; k < sizeof(hindex)/sizeof(*hindex) 
				&& i - hindex[k] >= 0; 	k++)
			acc += src[i - hindex[k]];
		dst[i] = acc / N_HARM;
	}
}


static inline void spes(float *dst, float src[][REAL_N_BANDS], float *max, int n)
{
	int i, t, f;
	float m = -INFINITY;
	
	for (f = 0; f < n; f++) {
		float acc = 0, tmp;
		for (t = 0; t < TIME_AVG; t++) {
			for (i = (-FREQ_AVG)/2 + 1; i <= FREQ_AVG/2; i++) {
				int fk = f + i;
				if (fk >= 0 && fk < n)
					acc += src[t][fk];
			}
		}
		tmp = acc / (TIME_AVG * FREQ_AVG);
		m = fmaxf(m, tmp);
		dst[f] = tmp;
	}
	*max = m;
}

static inline void npes(float *dst, float *src, int n)
{
	int f;
	float max = 0;
	
	for (f = 0; f < n; f++) {
		max = fmaxf(max, src[f]);
	}
	for (f = 0; f < n; f++) {
		dst[f] = src[f] - max;
	}
}

static int image_run(SDL_Surface *screen)
{
	const int W = screen->w, H = screen->h;
	float pesbuffer[TIME_AVG][REAL_N_BANDS];
	unsigned int bufindex = 0, read_p = 0;
	SDL_Surface *circ_buf;
	int k = 0, aa = 0, i;
	Uint32 last_time;
	float dbmin = INFINITY, dbmax = -INFINITY;
	
	uicontrol.base_band = REAL_N_BANDS - H;
	
	circ_buf = SDL_CreateRGBSurface(screen->flags, W, H, 
			screen->format->BitsPerPixel, 
			screen->format->Rmask,
			screen->format->Gmask,
			screen->format->Bmask,
			screen->format->Amask);
	
	SDL_FillRect(circ_buf, NULL, 0);
	
	read_p = ARTFI_DELAY - 1;
	
	for (i = 0; i < TIME_AVG*REAL_N_BANDS; i++)
		pesbuffer[0][i] = 0.0;
	
	last_time = SDL_GetTicks();
	uicontrol.running = 1;
	while (!uicontrol.quit_requested) {
		int read_valid = 0, y, f, retries = 0, dmode = uicontrol.mode;
		int baseb = uicontrol.base_band, paused = uicontrol.paused;
		float current[REAL_N_BANDS], spesbuf[REAL_N_BANDS], spesmax;
		SDL_Rect marker_present;
		SDL_Rect marker_present_ins = {0};
		SDL_Rect marker_past;
		Uint32 tmp_time;
		
		marker_present.h = marker_past.h = H;
		marker_present_ins.y = marker_present.y = marker_past.y = 0;
		marker_present.x = 0;
		
		
		while (!read_valid){
			int sv, i;
			
			retries++;
			if (retries > 1)
				PDEBUG("retries: %d\n", retries);
			
			sem_wait(block_lock);
			INCMOD(read_p, ARTFI_DELAY);
			
			sem_getvalue(block_lock, &sv);
			if (retries > 1)
				printf("sem: %d\n", sv);
			if (sv >= (ARTFI_DELAY - 1))
				continue;
			
			for (i = 0; i < REAL_N_BANDS; i++) {
				float tmp;
				tmp = dba(rtfi_blocks[read_p][i]);
				current[i] = denorm0(
					(dmode > ARTFI)? (tmp - iso226[i]) : tmp,
					PEAK_VALUE,
					LOWER_THRS);
			}
			
			sem_getvalue(block_lock, &sv);
			if (sv >= (ARTFI_DELAY - 1))
				continue;
			
			read_valid = 1;
		}
		aa++;
		if (!(aa%64)) {
			PDEBUG("min: %f, max: %f\n", dbmin, dbmax);
			dbmin = INFINITY;
			dbmax = -INFINITY;
		}
		
		marker_present.w = k + 1;
		marker_past.x = k + 1;
		marker_present_ins.x = marker_past.w = W - k - 1;
		
		if (!paused) 
		{
		pes(pesbuffer[bufindex], current, REAL_N_BANDS);
		spes(spesbuf, pesbuffer, &spesmax, REAL_N_BANDS);
	/*	if (dmode == NPES)
			npes(spesbuf, spesbuf, REAL_N_BANDS);
	*/	
		for (y = 0; y < H ; y++)
		{
			float tmp;
			float c;
			if ((f = y + baseb)< REAL_N_BANDS) {
				switch (dmode) {
				default:
				case NPES:
					c = denorm1(spesbuf[f], spesmax, 8);
					break;
				case SPES:
					c = denorm1(spesbuf[f], 40, 40);
					break;
				case PES:
					c = denorm1(pesbuffer[bufindex][f], 55, 50);
					break;
				case AES:
				case ARTFI:
					c = current[f]/LOWER_THRS;
					break;
				}
			} else {
				c = 0;
			}
			
			tmp = spesbuf[f];
			/*tmp = pesbuffer[bufindex][f];*/
			/* */
			dbmax = fmaxf(tmp, dbmax);
			dbmin = fminf(tmp, dbmin);
			putpixel(circ_buf, k, y, iris(c));
		}
		
		INCMOD(bufindex, TIME_AVG);
		
		k++;
		if (k >= W) {
			k = 0;
		}
		}
		tmp_time = SDL_GetTicks();
		if (tmp_time - last_time >= MIN_REFRESH_TIME) {
			if (!paused) {
			SDL_BlitSurface(circ_buf, &marker_present, screen, 
							&marker_present_ins);
			SDL_BlitSurface(circ_buf, &marker_past, screen, 
							&marker_present);
			}
			last_time = tmp_time;
			SDL_Flip(screen);
		}
		
		
	}
	uicontrol.running = 0;
	
	return 0;
}

static int event_parser(void *data)
{
	while (!uicontrol.quit_requested) {
		SDL_Event ev;
	/*	PDEBUG("aaa\n"); */
		SDL_WaitEvent(&ev);
		if (QuitFilter(&ev)) {
			uicontrol.quit_requested = 1;
		} else if (ev.type == SDL_KEYDOWN) {
			int ref_delta = 0, new_ref, mode_changed = 0;
			
			switch (ev.key.keysym.sym) {
			case REFLEVEL_PLUS:
				ref_delta = 1;
				break;
			case REFLEVEL_MINUS:
				ref_delta = -1;
				break;
			case PAUSE:
				uicontrol.paused = !uicontrol.paused;
				break;
			case MODE_PLUS:
				INCMOD(uicontrol.mode, N_MODES);
				mode_changed = 1;
				break;
			case MODE_MINUS:
				DECMOD(uicontrol.mode, N_MODES);
				mode_changed = 1;
				break;
			default: 
				continue;
			}
			
			if (mode_changed) {
				PDEBUG("Mode: %d\n" , uicontrol.mode);
				SDL_WM_SetCaption(modenames[uicontrol.mode],
						modenames[uicontrol.mode]);
			}
			
			if (ev.key.keysym.mod & REFLEVEL_ACCEL)
				ref_delta = ref_delta * ACCEL_AMOUNT;
			else
				ref_delta = ref_delta * NO_ACCEL_AMOUNT;
			
			new_ref = uicontrol.base_band + ref_delta;
			uicontrol.base_band = (new_ref > 0)? 
						((new_ref < REAL_N_BANDS)? 
							new_ref 
							: N_BANDS - 1)
						: 0;
		}
	}
	
	return 0;
}

