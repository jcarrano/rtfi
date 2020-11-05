#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  rtfi.py
#
#  Copyright 2012 Juan I Carrano <juan@carrano.com.ar>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.

import argparse
import os.path
import numpy as np
import scipy.signal as sig
from textwrap import wrap
from iso226 import iso226

# Resonator Time-Frequency Image
# This code calculates the parameters for a RTFI, as described in:
# [1] R. Zhou and M. Mattavelli, “A new time-frequency representation for music
#	signal analysis: resonator time-frequency image,” in Proceedings of
#	the 9th International Symposium on Signal Processing and Its
#	Applications (ISSPA ’07), Sharijah, UAE, February 2007.
# [2] R. Zhou, “Feature extraction of musical content for automatic music
#	transcription” Ph.D. dissertation, Swiss Federal Inst. of Technol.,
#	Lausanne, Oct. 2006 [Online].
#	Available: http://www.library.epfl.ch/en/theses/?nr=3638
# [3] R. Zhou and J. D. Reiss, “A real-time frame-based multiple pitch
#	estimation method using the resonator time-frequency image,” in
#	Proceedings of the 3rd Music Information Retrieval Evaluation eXchange
#	(MIREX ’07), Vienna, Austria, September 2007.
# [4] Ruohua Zhou, Joshua D. Reiss, Marco Mattavelli and Giorgio Zoia, "A
#	Computationally Efficient Method for Polyphonic Pitch Estimation",
#	EURASIP Journal on Advances in Signal Processing 2009, 2009 [Online].
#	Available: http://asp.eurasipjournals.com/content/2009/1/729494

def mtof(m):
	"""MIDI number to frequency."""
	return 440*2**((m - 69.0) / 12.0)

def const_q(n0, n1, fxst):
	"""
	Generate a list of center frequencies and bandwidth for a constant-Q filter
	bank.

	Parameters
	----------

	n0: lower frequency, given as MIDI number (i.e. 26 means 36.7Hz)
	n1: upper frequency, given as MIDI number (i.e. 116 means 6.6kHz)
	fxst: number of filters per semitone (10 is a good choice)

	Returns
	-------
	f0l: list of center frequencies in Hz (they will be space logarithmically)
	frw: list of bandwiths, in angular frequency (rad/s, or 2*pi*f)
	pitches: midi numbers corresponding to the frequencies in f0l.
	"""

	n0_m = n0 - 69
	n1_m = n1 - 69
	step = 1.0/fxst
	pitches = np.arange(n0, n1, step)

#	p0 = (pitches - 69.0) / 12.0
#	f0l = 440*2**(p0)
	f0l = mtof(pitches)

	d1 = 2**(1.0/(fxst*12))
	c = (2.0 * d1 - 2) / (d1 + 1)
	frw = f0l * 2 * np.pi * c

	return f0l, frw, pitches

def f2band(f0l, n_harm):
	"""F2BAND Generar indices de armónicos
	bandlist(i, j) devuelve el índice del canal donde se encuentra el j-ésimo armónico de f0l(i)
	En realidad debería considerar los errores relativos, pero como hay
	muchos bines muy cercanos tomo error absoluto que es más fácil."""
	bandlist = []
	maxf = f0l[-1]
	for f in f0l:
		targets = f * np.arange(1,n_harm+1);
		indices = [(abs(f0l - ft)).argmin() for ft in targets if ft <= maxf]
		bandlist.append(indices)
	return bandlist

def filter_coeffs(f0, fr_w, fs):
	"""Generate the coefficients of the complex resonator filter.

	The difference-equation for this filter is:
		y[n] = k * x[n] + a1 * y[n-1]

	Here k is a real gain and a1 is the complex parameter determining both the
	frequency and the decay time of the filter.

	Parameters
	----------

	f0: array of center frequencies (in Hz)
	fr_w: array of equivalent rectangular bandwidths in rad/s.
	fs: sample frequency in Hz.

	Returns
	-------

	k: array of gains (real)
	a1: resonant parameter (complex)
	"""
	fs = float(fs)
	w0 = f0 * 2 * np.pi
	ERB = fr_w
	r_wm = ERB/np.pi
	k = 1 - np.exp(-r_wm / fs)
	a1 = np.exp((-r_wm + 1j * w0) / fs)

	return a1, k

def resonator_run(a1, k, x):
	#y =
	pass

def decimator_design(f0, fr_w, fs, att, force_n = None):
	"""Design a low-pass decimating filter (using kaiser window) suitable for
	feeding into a filter bank.

	Given the frequencies and bandwidths of a filter bank, a filter is designed
	such that all components of the back fit within the pass band. It is assumed
	that during decimation the transition band with intersect with itself.

	Parameters
	----------

	f0: list of filterbank center frequencies (in Hz). Only the last element
		(highest frequency) will be considered.
	frw: list of bandwidths, in angular frequency. Only the last element will be
		considered.
	fs: sample frequency (in Hz).
	att: stopband attenuation in dB.
	force_n: Fix the order if the filter. If not set, the order will be determined
		automatically. If this is less than the required order, an error will
		be thrown.

	Returns
	-------

	fpass: end of the pass-band in Hz.
	fstop: beggining of the stop-band in Hz.
	h: impulse response.
	"""
	fpass = f0[-1] + frw[-1]/(2*np.pi)
	fstop = fs/2 - fpass

	n, beta = sig.kaiserord(att, 2*(fstop-fpass)/float(fs))

	if force_n is not None:
		if force_n < n:
			raise Exception('Required order is more than specified')
		else:
			n = force_n

	h = sig.firwin(n, (fstop+fpass)/2, fstop-fpass,'kaiser', nyq = fs/2)

	return fpass, fstop, h


FS = [44100, 48000, 96000] # sampling frequencies
FXST = 10 # filter per semitone
PINIT = 26 # initial midi#
PEND = 116 # end midi#
ATT = 96 #dB
OCTAVE = 12
BLOCK = FXST*OCTAVE
FILENAME = "rtfi_params.c"
AUXFILENAME = "rtfi_defines.h"
SPECFILE = "spectral_tables.c"
ISOPHON = 70.0
N_HARM = 10

file_header = """/*
 * %s
 *
 * Copyright 2012 Juan I Carrano <juan@carrano.com.ar>
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

/* ATTENTION:
	This file was automatically generated. DO NOT edit by hand. Refer to
	the rtfi.py program if you need to change the parameters.
*/

#include <complex.h>

const int RTFI_SAMPLERATES[%d] = { %s };

"""
struct_def = """
struct rtfi_param {
	float decfilter[DFILTER_N / 2]; /* we use an even, symmetric FIR */
	complex float a1[BLOCK];
	float k[BLOCK];
};
"""

param_template = """
static const struct rtfi_param rtfi_{fs!s} =
{{
	.decfilter = {{ {h}
	}},
	.a1 = {{ {a1}
	}},
	.k = {{ {k}
	}},
}};
"""

defines_header = """/* Automatically generated definitions. DO NOT edit */
#ifndef __{0}__
#define __{0}__
"""
defines_footer = """
#endif /* __%s__ */ /* End of automatically generated definitions */
"""

spec_header = """/* ISO226 Equal loudness contour.
Calculated for %f phon. Interpolated using cubic splines.
Automatically generated file. DO NOT edit
iso226[0] -> highest frequency */
"""

isoarray = """static const float iso226[N_BANDS] = {{
	{curve}
}};
"""

spec_footer = """
/* End of automatically generated file */
"""

hindex_template = """
static const int hindex[N_HARM - 1] = {{
	{hi}
}};
"""

def number2str(x):
	if not x.imag:
		if isinstance(x, np.integer):
			return str(x)
		else:
			return "%.16ff"%x
	else:
		return "%.16ff + %.16ff*I"%(x.real, x.imag)

def list2carray(l):
	"""Convert a numpy array into a C array for inclusion in a source file."""
	return "\n\t".join(wrap(", ".join(number2str(x) for x in l)))

def create_params(f0, frw, fs, att, n):
	upper_f0 = f0[-BLOCK:]
	upper_frw = frw[-BLOCK:]

	a1, k = filter_coeffs(upper_f0, upper_frw, fs/2)
	fpass, fstop, h = decimator_design(f0, frw, fs, att, n)

	if len(h) % 2:
		raise RuntimeError("h must have an even number of coefficients")

	h = h[:len(h)//2] # remove redundancies

	return param_template.format(fs = fs, h = list2carray(h),
				a1 = list2carray(a1), k = list2carray(k))

def normiso(f0):
	"""Get a iso226 curve normalized so that the minimum value is zero."""
	x = iso226(ISOPHON, f0)
	return x - min(x)

def create_isocurve(f0):
	"""Create a C array with the equal loudness contour for the frequencies in
	f0. The minimum value will be zero."""
	return isoarray.format(curve = list2carray(reversed(normiso(f0))))

def allequal(l):
	e = next(l)
	return all(e == x for x in l)

def hindexes(f0, nh):
	a1 = f2band(f0, nh)
	# check offsets are constant
	a1n = [[i - band[0] for i in band] for band in a1]
	checks = (allequal(band[k] for band in a1n if len(band) > k) for k in
							range(len(a1[0])))
	if all(checks):
		return a1n[0]
	else:
		raise RuntimeError('harmonic index check failed')

def create_hindexes(f0, nh):
	hi = hindexes(f0, nh)

	return hindex_template.format(hi = list2carray(hi[1:]))

def define(f, k, v):
	"""Write a preprocessor #define macro to the file object f."""
	f.write("#define {0} {1}\n".format(k, v))

def parse_args():
	parser = argparse.ArgumentParser(description="Generate (or plot) RTFI "
									 "filter parameters.")
	parser.add_argument("-p", "--plot", help="Plot frequency response",
						action="store_true")
	parser.add_argument("-w", "--write", help="Write output to file",
						action="store_true")

	parser.add_argument("-m", "--mainfile", help="Override filename for filter "
						"coefficient table.", default=FILENAME)
	parser.add_argument("-a", "--auxfile", help="Override filename for the header "
						"of coefficient table.", default=AUXFILENAME)
	parser.add_argument("-s", "--specfile", help="Override filename for the equal "
						"loudness contour table.", default=SPECFILE)


	return parser.parse_args()

if __name__ == '__main__':
	ns = parse_args()

	if ns.write:
		fo = open(ns.mainfile, 'w+')
		fd = open(ns.auxfile, 'w+')
		specf = open(ns.specfile, 'w+')
	else:
		import sys
		fo = fd = specf = sys.stdout

	auxfile_clean = ns.auxfile.replace('.', '_').replace('/', '_')
	fo.write(file_header % (ns.mainfile, len(FS), list2carray(FS)))

	if ns.write:
		fo.write('#include "%s"\n' % os.path.relpath(
			ns.auxfile, os.path.dirname(ns.mainfile)))
		fd.write(defines_header.format(auxfile_clean))

	define(fd, 'FXST', FXST)
	define(fd, 'OCTAVE', OCTAVE)
	define(fd, 'BLOCK', "(FXST*OCTAVE)")

	f0, frw, p = const_q(PINIT, PEND, FXST)

	nbands = len(f0)
	steps = int(np.ceil(nbands / float(BLOCK)))
	define(fd, 'RTFI_STEPS', steps)
	define(fd, 'N_BANDS', nbands)
	define(fd, 'REAL_N_BANDS', "((%d - %d) * FXST)" % (PEND, PINIT))
	define(fd, 'LOWF_IGNORE', "(N_BANDS - REAL_N_BANDS)")

	maxbands = steps * BLOCK
	bottom_minindex = excess = maxbands - nbands
	define(fd, 'BOTTOM_MINIDEX', bottom_minindex)

	minfs = min(FS)
	ma1, mk = filter_coeffs(f0, frw, minfs)
	h = decimator_design(f0, frw, minfs, ATT)[-1]
	#We designing the filter for the smallest Fs, for the other cases,
	#the filter order will be smaller
	maxn = len(h)

	define(fd, 'DFILTER_N', maxn)

	if ns.write:
		fd.write(defines_footer % auxfile_clean)

	fo.write(struct_def)

	for fs in FS:
		fo.write(create_params(f0, frw, fs, ATT, maxn))

	fo.write("/* Automatically generated file ends here */\n")

	specf.write(spec_header % ISOPHON)
	if ns.write:
		specf.write('#include "%s"\n' % os.path.relpath(
			ns.auxfile, os.path.dirname(ns.specfile)))

	define(specf, 'N_HARM', N_HARM);
	specf.write(create_hindexes(f0, N_HARM))
	specf.write(create_isocurve(f0))
	specf.write(spec_footer)

	if ns.plot:
		import matplotlib.pyplot as plt

		plt.figure()
		plt.stem(range(len(h)), h)

		plt.figure()

		# let's illustrate the recursive decimation & filtering process
		for i in range(steps):
			current_fs = (0.0 + minfs)/(2**i)
			w, H = sig.freqz(h, 1, 2048, whole = 0)
			plt.plot(current_fs/2 * w/np.pi, 20*np.log10(abs(H)))

			band_top = -(BLOCK*i + 1)
			band_bottom = band_top - BLOCK + 1

			a1_top = ma1[band_top]
			k_top = mk[band_top]

			try:
				a1_bott = ma1[band_bottom]
			except IndexError:
				band_bottom = 0;
				a1_bott = ma1[band_bottom]

			k_bott = mk[band_bottom]

			w2, H2 = sig.freqz(k_top, np.array([1, (-a1_top)]), 50000, whole = 0)
			w3, H3 = sig.freqz(k_bott, np.array([1, (-a1_bott)]), 50000, whole = 0)
			p1 = plt.plot(minfs/2 * w2/np.pi, 20*np.log10(abs(H2)))[0]
			plt.plot(minfs/2 * w3/np.pi, 20*np.log10(abs(H3)), color = p1.get_color())

		plt.plot(f0, normiso(f0))

		plt.figure()

		f, H = sig.freqz(h, 1, 2048, whole = 0)
		f *= (FS[0]/2)/np.pi

		plt.subplot(3,1,1)
		plt.plot(f, np.abs(H))
		plt.title('magnitude')

		plt.subplot(3,1,2)
		plt.plot(f, np.unwrap(np.angle(H)))
		plt.title('phase')

		plt.subplot(3,1,3)
		plt.plot(f, np.unwrap(np.angle(H))/(f*2*np.pi))
		plt.title('delay')

		plt.figure()



		plt.show()
