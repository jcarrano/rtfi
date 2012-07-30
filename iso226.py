#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  untitled.py
#  
#  Copyright 2012 Juan I Carrano <juan@superfreak.com.ar>
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

import numpy as np
from scipy.interpolate import interp1d

f = np.array([20, 25, 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400,
	500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300,
	8000, 10000, 12500])

af = np.array([0.532, 0.506, 0.480, 0.455, 0.432, 0.409, 0.387, 0.367, 0.349,
	0.330, 0.315, 0.301, 0.288, 0.276, 0.267, 0.259, 0.253, 0.250, 0.246, 
	0.244, 0.243, 0.243, 0.243, 0.242, 0.242, 0.245, 0.254, 0.271, 0.301])

Lu = np.array([-31.6, -27.2, -23.0, -19.1, -15.9, -13.0, -10.3, -8.1, -6.2,
	-4.5, -3.1, -2.0, -1.1, -0.4, 0.0, 0.3, 0.5, 0.0, -2.7, -4.1,
	-1.0, 1.7, 2.5, 1.2, -2.1, -7.1, -11.2, -10.7, -3.1])

Tf = np.array([78.5, 68.7, 59.5, 51.1, 44.0, 37.5, 31.5, 26.5, 22.1, 17.9, 14.4,
	11.4, 8.6, 6.2, 4.4, 3.0, 2.2, 2.4, 3.5, 1.7, -1.3, -4.2, -6.0, -5.4,
	-1.5, 6.0, 12.6, 13.9, 12.3])

def iso226(phon, freqs = None):
	"""Generates an Equal Loudness Contour as described in ISO 226
	
	This code is a translation of a Matlab function by Jeff Tacket.
	Additionally, this version does a cubic spline interpolation if it is 
	passed a vector of frequencies.
	The original documentation is reproduced below:
	
	function [spl, freq] = iso226(phon):
	
	Usage:  [SPL FREQ] = ISO226(PHON);
	
		PHON is the phon value in dB SPL that you want the equal
		loudness curve to represent. (1phon = 1dB @ 1kHz)
		SPL is the Sound Pressure Level amplitude returned for
		each of the 29 frequencies evaluated by ISO226.
		FREQ is the returned vector of frequencies that ISO226
		evaluates to generate the contour.
	
	Desc:   This function will return the equal loudness contour for
		your desired phon level.  The frequencies evaulated in this
		function only span from 20Hz - 12.5kHz, and only 29 selective
		frequencies are covered.  This is the limitation of the ISO
		standard.
	
		In addition the valid phon range should be 0 - 90 dB SPL.
		Values outside this range do not have experimental values
		and their contours should be treated as inaccurate.
	
		If more samples are required you should be able to easily
		interpolate these values using spline().
	
	Author: Jeff Tackett 03/01/05 """


	# TABLES FROM ISO226 
	# (they were moved outside of the function)  

	#Error Trapping
	if phon < 0 or  phon > 90:
		raise ValueError('Phon value out of bounds!')
	else:
		# Setup user-defined values for equation
		Ln = phon;
		
		# Deriving sound pressure level from loudness level (iso226 sect 4.1)
		Af=4.47E-3 * (10**(0.025*Ln) - 1.15) + (0.4*10**(((Tf+Lu)/10)-9 ))**af;
		Lp=((10/af)*np.log10(Af)) - Lu + 94;
		
		#Return user data
		if freqs is None:
			return f, Lp;
		else:
			return interp1d(f, Lp, 'cubic')(freqs)
