/*
 * rtfi.h
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

#ifndef _RTFI_H_
#define _RTFI_H_

#include "../src_generated/rtfi_defines.h"

#define BLK_SIZE_MS 10

/* Maximum lag (in blocks) between the graphical thread and the audio thread,
 * plus one,
 * Of course the minimum value is 1 + 1 = 2 */
#define ARTFI_DELAY 6
#define ARTFI_BSIZE N_BANDS

/* Macros to advance or recede an index in a circular array. Use with caution */
#define INCMOD(v, m) v = (v + 1) % (m)
#define DECMOD(v, m) v = (v - 1) % (m)

extern void *rtfi_prepare(int *ecode, sem_t *sem);
extern int rtfi_launch(void *client);
extern void rtfi_unload(void *client);

/* rtfi_blocks[?][0] : highest frequency
 * rtfi_blocks[?][ARTFI_BSIZE-1] : lowest frequency */
extern float rtfi_blocks[ARTFI_DELAY][ARTFI_BSIZE];
extern sem_t *block_lock;

#endif /* _RTFI_H_ */
