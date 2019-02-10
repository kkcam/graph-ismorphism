/*  This file is part of Conauto.
 
 Conauto is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 Conauto is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with Conauto.  If not, see <http://www.gnu.org/licenses/>.
 */

/* mismatch structure and functions for manage them */

#ifndef _MISMATCH_H_
#define _MISMATCH_H_

#include <stdlib.h>
#include <inttypes.h>

uint16_t mismatch_found_hash;

typedef struct {
	uint16_t *mismatch_hash;
	uint16_t *counters;
	uint16_t nom;	/* number of mismatches */
} MismatchCollection;

#include "seqpart.h"
#include "orbits.h"

void mark_new_mismatch ( Partition *p, uint8_t *mm_marked, Info_Vertex *info_vert, uint16_t mmv );

void mark_old_mismatch ( Partition *p, uint8_t *mm_marked, Info_Vertex *info_vert, uint16_t mmv );

uint16_t get_most_repeated_mismatch_vertex ( MismatchCollection *mmc );

void compute_mismatches ( MismatchCollection *mmc );

uint8_t mismatch_in_collection ( MismatchCollection *mmc, uint16_t *counters, uint16_t orbit_size );

#endif

