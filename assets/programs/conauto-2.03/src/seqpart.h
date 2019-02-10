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

/* data structures and function to generate sequences of partitions */

#ifndef _SEQPART_H_
#define _SEQPART_H_

#include <stdint.h>

#include "graph.h"
#include "groups.h"
#include "mismatches.h"

#define SET 0
#define VERTEX 1
#define BACKTR 2
#define UNKNOWN 3
#define LAST 4

#define TRUE 1
#define FALSE 0

#define EXCHANGE( a, b, type ) { type aux; aux=a; a=b; b=aux; }

typedef struct{
	uint16_t* vertices;
	uint16_t* belongs;
	uint16_t* start;
	uint64_t* attrib;
	uint64_t* adeg;
	MismatchCollection* mm_collection;
	uint8_t* affected;
	uint16_t* discarded_cells;
	Graph_Orbits *go;
	int backtrack_level;
	int auto_search_limit;
	uint16_t pivot;
	uint16_t noc; /* number of cells in this partition */
	uint16_t nodc; /* number of discarded cells */
	uint16_t nopdv; /* number of previously discarded vertices */
	uint16_t no;	/* number of orbits */
	uint8_t  tor; /* type of refinement applied */
} Partition;

typedef struct {
	Partition* partition;
	int lp; /* last partition in the sequence */
	uint16_t nodv; /* number of discarded vertices */
	uint16_t* discarded_vert; /* vertices that have been discarded */
	uint8_t* valid_new;
	uint8_t* valid_old;
	uint64_t *num_adj;
	uint32_t nodes;
	uint32_t nodes_limit;
	uint32_t dcs_nodes;
	uint32_t bad_nodes;
} SeqPart;

void free_partitions ( SeqPart *sp, int level );

void allocate_partition ( SeqPart *sp, int level, uint16_t noc, uint16_t num_vert );

uint16_t best_pivot ( const struct graph *g, SeqPart *sp, int level, uint8_t choose_not_valid );

int is_subpartition ( SeqPart *sp, int level, int bl );

void free_seqpart ( SeqPart *sp );

void allocate_seqpart ( SeqPart *sp, uint32_t size );

void generate_degree_partition ( SeqPart *sp, const struct graph *g );

void exchange_partitions ( Partition *p1, Partition *p2 );

void generate_seqpart ( SeqPart *sp, const struct graph *g, int level );

int degree_partitions_are_compatible ( Partition *p1, Partition *p2 );

void compute_auto_search_limits ( SeqPart *sp );

void compute_backtrack_levels ( SeqPart *sp );

uint16_t num_backtrack_levels ( SeqPart *sp );

void find_automorphisms ( const struct graph *g, SeqPart *sp, Perm_Group *pg );

int match ( int level, uint16_t *old_vertices, const struct graph *h, Perm_Group *pg, const struct graph *g, SeqPart *sp, Graph_Orbits *go, uint16_t *iso, uint16_t *perm_sets, uint16_t *leaders );

#endif

