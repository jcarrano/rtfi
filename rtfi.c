/*
 * rtfi.c
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
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <semaphore.h>
#include <jack/jack.h>
#include <libjc/common.h>
#include "rtfi_params.c"
#include "rtfi.h"

#define DIVUP(a, b) ((((a) - 1) / (b)) + 1)

#define CLIENTNAME "RTFI"

typedef jack_default_audio_sample_t sample_t;

/* These are circular buffers used to store the last samples, in order to 
 * compute the decimating FIR across each block */
#define CB_LEN DFILTER_N
static sample_t last_samples[RTFI_STEPS][CB_LEN] = {{0}};
/* the cb_states are 1 + index of latest sample. That means the are initialized
 * to CB_LEN, or 0, as they wrap around CB_LEN */
static int cb_states[RTFI_STEPS];

/* Working area */
static sample_t *decbuf = NULL;
static complex float yprev[RTFI_STEPS][BLOCK];
static complex float partial_power[RTFI_STEPS][BLOCK];
static int block_avg_nsamples[RTFI_STEPS];
static const struct rtfi_param *rtfi_cfg;

/* we use this counter in case  we have to calculate each 2^n frames, instead
 * of every frame. This happens if jack's buffer size is less than 2^STEPS, the
 * lasts steps don't produce outputs in every call to the process callback 
 * We don't care if frame count overflows */
static int frame_count;
/* Remainig samples to be processed before completing the ARTFI block*/
static int partial_rem;
/* Each ARTFI block is made by processing block_input_len samples and averaging
 * the outputs */
static int block_input_len;

static jack_nframes_t sr;
static jack_port_t* inp;

/* Communication */
float rtfi_blocks[ARTFI_DELAY][ARTFI_BSIZE];
sem_t *block_lock;
static int b_write;
/* int b_read; */ /* Let the reader take care of the read index */

static inline int min(int a, int b)
{
	return (a < b)? a : b;
}

static inline sample_t cbuf_address(sample_t *cb, int cb_state, int delay)
{ /* DELAY MUST BE >= 1 !!!!!!!!!!!!!!!!!!*/
	return cb[(cb_state - delay) & (CB_LEN - 1)]; /*"%" retains sign, so it
							is not useful */
}

static inline int cbuf_copy(sample_t *cb, int cb_state, sample_t *src, int n_samples)
{ /* Copy n_samples into the circular buffer (n_samples <= CB_LEN !!!!!)
	The only possibilities are n_samples = DFILTER_N, 
	or nsamples = DFILTER_N/(2^n). In the latter, cb_state can only be
	k*2^n
   returns the new value of cb_state
  */
  memcpy(cb + cb_state % CB_LEN, src, n_samples*sizeof(*cb));
  return (cb_state + n_samples) % CB_LEN;
}

static inline sample_t choose_src(sample_t *cb, int cb_state, sample_t *src, int i)
{ /* Get a sample either from the current block (src), or the saved block,
	dependig on the index */
	return (i < 0)? cbuf_address(cb, cb_state, -i) : src[i];
}

static inline int decimate(const float *h, sample_t *cb, int cb_state, sample_t *src,
						int n_samples, sample_t *dst)
{ /* We are going to assume that the decimating filter is simmetric and has an
	even number of coefficients. In fact, if DFILTER_N is not a power of 2,
	expect everything to fall apart
	n_samples is the number of INPUT samples to process
	the number of samples produced is (n_samples+1)/2
	At the end of the process, the last samples of SOURCE are saved*/
	const int first_stage_l = min(DFILTER_N - 1, n_samples);
	const int to_copy = min(DFILTER_N, n_samples);
	int n;
/*	
	for (n = 0; n < first_stage_l; n += 2) {
		int i;
		sample_t acc = 0;
		
		for (i = 0; i < (DFILTER_N / 2); i++) {
			acc += h[i] * (choose_src(cb, cb_state, src, n-i) 
			   + choose_src(cb, cb_state, src, (n+i) - (DFILTER_N - 1)));
		}
		
		dst[n/2] = acc;
	}
	
	for ( ; n < n_samples; n += 2) {
		int i;
		sample_t acc = 0;
		
		for (i = 0; i < (DFILTER_N / 2); i++) {
			acc += h[i] * (src[n-i] + src[(n+i) - (DFILTER_N - 1)]);
		}
		
		dst[n/2] = acc;
	}
*/	

	for (n = 0; n < n_samples; n += 2) {
		int i;
		sample_t acc = 0;
		
		for (i = 0; i < (DFILTER_N / 2); i++) {
			acc += h[i] * (choose_src(cb, cb_state, src, n-i) 
			   + choose_src(cb, cb_state, src, (n+i) - (DFILTER_N - 1)));
		}
		
		dst[n/2] = acc;
	}
	
	return cbuf_copy(cb, cb_state, src + n_samples - to_copy, to_copy);
}

static inline int KTH_BUFSIZE(int bs, int k)
{ /* Minimum buffer size is 1.*/
	int kbs = bs >> (k+1);
	return kbs? kbs : 1;
}

static inline int decbuf_index(int jack_bufsize, int k)
{ /* Return the offset of the first element of the section of "decbuf"
	corresponding to step k ( 0 <= k < RTFI_STEPS )*/
	int i, acc = 0;
	
	for (i = 0; i < k; i++) {
		acc += KTH_BUFSIZE(jack_bufsize, i);
	}
	
	return acc;
}

static inline float abs2(float complex z)
{
	return crealf(z)*crealf(z) + cimagf(z)*cimagf(z);
}

static inline int STEP_RUNNABLE(int bs, int step, int frame_count)
{
	int skip_n = (step - __builtin_clz(bs) + 1); /* 0 means do always, 1
			 do each 2 blocks, 2 do each 4 blocks, etc */
	return !(frame_count % (1 << ((skip_n <= 0) ? 0 : skip_n)));
}

static int rtfi_process(jack_nframes_t nframes, void *arg)
{
	sample_t *inb;
	int step, pstep, processed = 0;
	const complex float *a1 = rtfi_cfg->a1;
	const float *k = rtfi_cfg->k;
	
	inb = jack_port_get_buffer(inp, nframes);
	
	for (step = 0; step < RTFI_STEPS; step++) {
		int n_samples_in = KTH_BUFSIZE(nframes, step-1);
	/*	int n_samples_out = KTH_BUFSIZE(nframes, step); */
		sample_t *src, *dst;
		
		src = (step)? (decbuf + decbuf_index(nframes,step-1)): inb;
		dst = (decbuf + decbuf_index(nframes,step));
		
		if (!STEP_RUNNABLE(nframes, step, frame_count)) {
			cb_states[step] = cbuf_copy(last_samples[step],
						     cb_states[step], src, 1);
			break;
		} else {
			cb_states[step] = decimate(rtfi_cfg->decfilter, 
				last_samples[step], cb_states[step], src, 
							n_samples_in, dst);
		}
	}

#define ARTFI_LOC(step, blockn) (step * BLOCK + (BLOCK - 1 - blockn))

	while (processed < nframes) {
		int to_process = min(nframes - processed, partial_rem);
		
		for (pstep = 0; pstep < RTFI_STEPS 
				&& STEP_RUNNABLE(nframes, pstep, frame_count); 
								pstep++) {
			int bk, iinit, iend;
			sample_t *src = (decbuf + decbuf_index(nframes,pstep));
				
			iinit = DIVUP(processed, 2 << pstep);
			iend = DIVUP(processed + to_process, 2 << pstep);
			
			if (partial_rem == block_input_len)
				block_avg_nsamples[pstep] = (iend - iinit);
			else
				block_avg_nsamples[pstep] += (iend - iinit);
			
			for (bk = 0; bk < BLOCK; bk++) {
				int i;
				complex float y = yprev[pstep][bk];
				float yacc = 0;
				
				for (i = iinit; i < iend; i++) {
					y = k[bk] * src[i] + a1[bk] * y;
				/*	y = src[i]; */
					yacc += abs2(y);
				}
				
				yprev[pstep][bk] = y;
				/* Could we have that for some artfi block, some
				 * pstep produces no samples to average?
				 * Not with current settings. We don't care */
			/*	rtfi_blocks[b_write][ARTFI_LOC(pstep, bk)] = 
					((iend-iinit)?
						yacc / (iend - iinit)
						: 0
					)
				     +  ((partial_rem == block_input_len)?
							0
						:rtfi_blocks[b_write][ARTFI_LOC(pstep, bk)]
					); */
				rtfi_blocks[b_write][ARTFI_LOC(pstep, bk)] = 
					(yacc + 
					((partial_rem == block_input_len)?
							0
						:rtfi_blocks[b_write][ARTFI_LOC(pstep, bk)]
					))
					/ ((partial_rem == to_process)?
						block_avg_nsamples[pstep]
						:1);
			}
		}
		
		partial_rem -= to_process;
		processed += to_process;
		if (partial_rem == 0) {
			INCMOD(b_write, ARTFI_DELAY);
			partial_rem = block_input_len;
			sem_post(block_lock);
		}
	}
	
	frame_count++;	
	return 0;
}

static inline int decbuf_minsize(int jack_bufsize) {
	return decbuf_index(jack_bufsize, RTFI_STEPS);
}

void *rtfi_prepare(int *ecode, sem_t *sem)
{ /* Returns a jack client on success, NULL on failure, error code in *ecode */
	jack_client_t* client;
	int bs, dbs, i;
	int r = 0;
	
	/* Jack initialization */
	client = jack_client_open(CLIENTNAME, JackNullOption, NULL);
	if (client == NULL) {
		PERROR("Could not register client\n");
		r = -E_OTHER;
		goto disaster;
	}
	
	inp = jack_port_register(client, "Input",
			JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
	
	jack_set_process_callback(client, rtfi_process, NULL);
	
	bs = jack_get_buffer_size(client);
	dbs = decbuf_minsize(bs);
	printf("%d X %d => %d\n", bs, RTFI_STEPS, dbs);
	sr = jack_get_sample_rate(client);
	
	switch (sr) {
		case 44100:
			rtfi_cfg = &rtfi_44100;
			break;
		case 48000:
			rtfi_cfg = &rtfi_48000;
			break;
		case 96000:
			rtfi_cfg = &rtfi_96000;
			break;
		default:
			goto disaster;
	}
	
	block_input_len = ceilf((sr * BLK_SIZE_MS) / 1000.0);
	partial_rem = block_input_len;
	
	if (NMALLOC(decbuf, dbs) == NULL)
		r = -E_NOMEM;
		
	block_lock = sem;
	
	for (i = 0; i < ARSIZE(yprev); i++)
		yprev[0][i] = 0;
	
	for (i = 0; i < ARSIZE(partial_power); i++)
		partial_power[0][i] = 0;
	
/* Leave activation to the caller */	
/*	if (jack_activate(client)) {
		printf("Could not activate client\n");
		r = 3;
		goto disaster;
	}
*/
disaster:
	if (r != 0) {
		rtfi_unload(client);
		client = NULL;
	}
	if (ecode != NULL)
		*ecode = r;
	
	return client;
}

void rtfi_unload(void *client)
{
	jack_client_close(client);
	free(decbuf);
	decbuf = NULL;
}

int rtfi_launch(void *client)
{
	return jack_activate(client);
}
