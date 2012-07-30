/*
 * mkmono.c
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
#include <unistd.h>
#include <jack/jack.h>

#define CLIENTNAME "Mono Mixer"

typedef jack_default_audio_sample_t sample_t;

static jack_port_t* inr;
static jack_port_t* inl;
static jack_port_t* outp;

int process(jack_nframes_t nframes, void *arg)
{
	sample_t *inbr, *inbl, *outb;
	int i;
	
	inbr = jack_port_get_buffer(inr, nframes);
	inbl = jack_port_get_buffer(inl, nframes);
	outb = jack_port_get_buffer(outp, nframes);
	
	for (i = 0; i < nframes; i++) {
		outb[i] = (inbr[i] + inbl[i]) / 2;
	}
	
	return 0;
}


int main(int argc, char **argv)
{
	int r;
	
	if (argc == 2 && argv[1][0] == 'h') {
		printf("%s by jcarrano\n", argv[0]);
		printf("Use with the jack server\n");
		r = 1;
	} else {
		jack_client_t* client;
		
		client = jack_client_open(CLIENTNAME, JackNullOption, NULL);
		if (client == NULL) {
			printf("Could not register client\n");
			r = 2;
			goto disaster;
		}
		
		inl = jack_port_register(client, "Left",
				JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
		inr = jack_port_register(client, "Right",
				JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
		outp = jack_port_register(client, "Mixed", 
				JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
		
		jack_set_process_callback(client, process, NULL);
		
		if (jack_activate(client)) {
			printf("Could not activate client\n");
			r = 3;
			goto disaster;
		}
	}
	
	while (1) {
		sleep(2);
	/*	printf("%f\n", maxvalue); */
	}
	
disaster:
	return r;
}
