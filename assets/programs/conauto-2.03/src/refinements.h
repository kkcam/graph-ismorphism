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

/* refinements applied to partitions */

#ifndef _REFINEMENTS_H_
#define _REFINEMENTS_H_

#include <stdint.h>

#include "mismatches.h"
#include "graph.h"
#include "seqpart.h"
#include "orbits.h"

void set_refinement ( SeqPart *sp, const struct graph *g, int level, uint8_t *success );

void vertex_refinement ( SeqPart *sp, const struct graph *g, int level );

uint8_t set_ref_compat ( SeqPart *sp, const struct graph *g, int level, uint16_t *new_vertices, uint16_t *old_vertices, uint16_t *discarded_vert );

uint8_t vertex_ref_compat ( SeqPart *sp, const struct graph *g, int level, uint16_t p, uint16_t *new_vertices, uint16_t *old_vertices, uint16_t *discarded_vert );

uint8_t last_part_compat ( Partition *p, uint16_t *vertices, const struct graph *g, const struct graph *h );

int multirefinement ( int level, SeqPart *sp, const struct graph *g );

#endif
