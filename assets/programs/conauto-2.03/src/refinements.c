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

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "refinements.h"
#include "sort.h"
#include "orbits.h"
#include "seqpart.h"
#include "mismatches.h"

const uint64_t adjacency[4] = { UINT64_C(0), UINT64_C(0x100000001), UINT64_C(0x10001), UINT64_C(0x1000000000001) };

#define ADD_DISCARDED_VERT( vertices, dv ) \
{	uint16_t v; \
	for ( v = sp->partition[level].start[c]; v < sp->partition[level].start[c+1]; v++ ) \
		dv[sp->nodv++] = vertices[v]; \
}

#define COMPUTE_NUM_ADJ( vertices ) \
{	uint16_t v, pv; \
	for ( v = sp->partition[level].start[c]; v < sp->partition[level].start[c+1]; v++ ) \
	{ \
		sp->num_adj[vertices[v]] = 0; \
		for ( pv = sp->partition[level].start[pc]; pv < sp->partition[level].start[pc+1]; pv++ ) \
			sp->num_adj[vertices[v]] += UINT64_C(1) << ( g->adj[vertices[pv]][vertices[v]] << 4 ); \
	} \
}

#define COPY_CELL( old_vertices, new_vertices ) \
	start = sp->partition[level+1].start[nc]; \
	target_vert = &(new_vertices[start]); \
	memcpy ( target_vert, &(old_vertices[sp->partition[level].start[c]]), size*sizeof(uint16_t) );

#define CHARACTERIZE_CELL \
	sp->partition[level+1].adeg[nc] = adeg; \
	sp->partition[level+1].attrib[nc] = sp->partition[level].attrib[c];

void set_refinement ( SeqPart *sp, const struct graph *g, int level, uint8_t *success )
{
	/* level identifies the partition to be refined */
	const uint16_t pc = sp->partition[level].pivot; /* pivot cell */
	uint16_t nc; /* number of cells in the new partition */
	uint16_t c; /* cell being processed */
	uint16_t size; /* size of cell c */
	uint16_t start; /* start point of the next cell in the new partition */
	uint16_t *target_vert; /* vertices of the cell being processed */
	uint64_t adeg; /* available degree with respect to the pivot cell */
	uint16_t split; /* the cell just processed has been split */
	uint16_t v; /* the vertices in the cell being processed */

	nc = 0;
	sp->partition[level].nodc = 0;
	sp->partition[level+1].start[nc] = 0;
	*success = FALSE;
	for ( c = 0; c < sp->partition[level].noc; c++ )
	{
		if ( (uint16_t)sp->partition[level].attrib[c] == 0 )
		{
			sp->partition[level].discarded_cells[sp->partition[level].nodc++] = c;
			ADD_DISCARDED_VERT(sp->partition[level].vertices, sp->discarded_vert);
			continue;
		}
		COMPUTE_NUM_ADJ(sp->partition[level].vertices);
		size = (uint16_t) ( sp->partition[level].start[c+1] - sp->partition[level].start[c] );
		COPY_CELL( sp->partition[level].vertices, sp->partition[level+1].vertices );
		sort_cell ( size, target_vert, sp->num_adj );
		adeg = sp->num_adj[target_vert[0]];
		CHARACTERIZE_CELL;
		sp->partition[level+1].belongs[target_vert[0]] = nc;
		nc++;
		split = FALSE;
		for ( v = 1; v < size; sp->partition[level+1].belongs[target_vert[v]] = (uint16_t) (nc-1), v++ )
		{
			if ( adeg != sp->num_adj[target_vert[v]] )
			{
				sp->partition[level+1].start[nc] = (uint16_t) ( start + v );
				adeg = sp->num_adj[target_vert[v]];
				sp->valid_new[nc-1] = TRUE;
				CHARACTERIZE_CELL;
				nc++;
				*success = TRUE;
				split = TRUE;
			}
			sp->partition[level+1].belongs[target_vert[v]] = (uint16_t) ( nc - 1 );
		}
		sp->partition[level+1].start[nc] = (uint16_t) ( start + size );
		sp->partition[level].affected[c] = ( split || ( ( adeg & UINT64_C(0xFFFFFFFFFFFF0000) ) != 0 ) );
		sp->valid_new[nc-1] = sp->valid_old[c] || split;
	}
	sp->partition[level+1].noc = nc;
	sp->partition[level+1].nopdv = sp->nodv;
}

#define COMPUTE_COUNTERS_AND_POSITIONS( VERTICES ) \
	for ( adj = NOT_ADJ; adj <= ARC_IO; counters[adj++] = 0 ); \
	for ( v = sp->partition[level].start[c]; v < sp->partition[level].start[c+1]; v++ ) \
		if ( p != VERTICES[v] ) \
			counters[g->adj[p][VERTICES[v]]]++; \
	for ( adj = ARC_IO, positions[ARC_IO] = sp->partition[level+1].start[nc]; adj > NOT_ADJ; adj-- ) \
		positions[adj-1] = (uint16_t) ( positions[adj] + counters[adj] );

void vertex_refinement ( SeqPart *sp, const struct graph *g, int level )
{
	const uint16_t pc = sp->partition[level].pivot; /* pivot cell */
	const uint16_t p = sp->partition[level].vertices[sp->partition[level].start[pc]];  /* pivot vertex */
	uint16_t nc; /* number of cells in the new partition */
	uint16_t c; /* cell being processed */
	uint16_t counters[4]; /* number of vertices for each adjacency type */
	uint16_t positions[4]; /* position of new cells in the new partition */
	uint16_t corr_cell[4]; /* corresponding cell */
	uint16_t v; /* the vertices in the cell being processed */
	int8_t adj; /* adjacency information of new cells */
	uint16_t size; /* size of the cell being refined */
	
	nc = 0;
	sp->partition[level].nodc = 0;
	sp->partition[level+1].start[nc] = 0;
	sp->discarded_vert[sp->nodv++] = p;
	for ( c = 0; c < sp->partition[level].noc; c++ )
	{
		if ( (uint16_t)sp->partition[level].attrib[c] == 0 )
		{
			sp->partition[level].discarded_cells[sp->partition[level].nodc++] = c;
			ADD_DISCARDED_VERT(sp->partition[level].vertices, sp->discarded_vert);
			sp->partition[level].affected[c] = FALSE;
			continue;
		}
		size = (uint16_t) ( sp->partition[level].start[c+1] - sp->partition[level].start[c] );
		COMPUTE_COUNTERS_AND_POSITIONS ( sp->partition[level].vertices );
		for ( adj = ARC_IO; adj >= NOT_ADJ; adj-- )
			if ( counters[adj] > 0 )
			{
				sp->partition[level+1].attrib[nc] = sp->partition[level].attrib[c] - adjacency[adj];
				sp->valid_new[nc] = sp->partition[level+1].attrib[nc] && ( sp->valid_old[c] || ( counters[adj] < size ) );
				corr_cell[adj] = nc;
				nc++;
				sp->partition[level+1].start[nc] = (uint16_t) ( positions[adj] + counters[adj] );
			}
		sp->partition[level].affected[c] = ( counters[NOT_ADJ] < size );
		for ( v = sp->partition[level].start[c]; v < sp->partition[level].start[c+1]; v++ )
			if ( p != sp->partition[level].vertices[v] )
			{
				sp->partition[level+1].vertices[positions[g->adj[p][sp->partition[level].vertices[v]]]++] = sp->partition[level].vertices[v];
				sp->partition[level+1].belongs[sp->partition[level].vertices[v]] = corr_cell[g->adj[p][sp->partition[level].vertices[v]]];
			}
	}
	sp->partition[level+1].noc = nc;
	sp->partition[level+1].nopdv = sp->nodv;
}

uint8_t set_ref_compat ( SeqPart *sp, const struct graph *g, int level, uint16_t *new_vertices, uint16_t *old_vertices, uint16_t *discarded_vert )
{
	const uint16_t pc = sp->partition[level].pivot; /* pivot cell */
	uint16_t c; /* cell being processed */
	uint16_t nc; /* number of validated cells */
	uint16_t start;
	uint16_t size;
	uint16_t *target_vert;
	uint16_t v;
	uint8_t compat;
#ifdef RF
 	uint16_t hash_size;
 	uint16_t hi; /* hash index */
 
 	for ( hash_size = 0, c = 0; c < sp->partition[level].noc; c++ )
 		if ( sp->partition[level].affected[c] )
 			hash_size = (uint16_t)(hash_size + sp->partition[level].start[c+1] - sp->partition[level].start[c]);
 	{ /* new scope for variables */
 	uint16_t hash_vector[2+hash_size*5];
 	hi = 0;
 	hash_vector[hi++] = (uint16_t) level;
 	hash_vector[hi++] = (uint16_t)(level >> 16);
 	hi = (uint16_t)(hi - ( (level >> 16) == 0 ));
#endif
 	compat = TRUE;
	sp->nodv = sp->partition[level].nopdv;
	nc = 0;
	for ( c = 0; c < sp->partition[level].noc; c++ )
	{
		if ( (uint16_t)sp->partition[level].attrib[c] == 0 )
		{
			ADD_DISCARDED_VERT( old_vertices, discarded_vert );
			continue;
		}
		size = (uint16_t) ( sp->partition[level].start[c+1] - sp->partition[level].start[c] );
		COPY_CELL( old_vertices, new_vertices );
		if ( ! sp->partition[level].affected[c] )
		{
			nc++;
			continue;
		}
		COMPUTE_NUM_ADJ(old_vertices);
		sort_cell ( size, target_vert, sp->num_adj );
		for ( v = 0; v < size; v++ )
			if ( sp->num_adj[target_vert[v]] != sp->partition[level+1].adeg[sp->partition[level+1].belongs[sp->partition[level+1].vertices[start+v]]] )
			{
#ifdef RF
				compat = FALSE;
                                hash_vector[hi++] = v;
                                hash_vector[hi++] = (uint16_t)(sp->num_adj[target_vert[v]] + 9173);
                                hash_vector[hi++] = (uint16_t)( (sp->num_adj[target_vert[v]] >> 16) + 9173);
                                hash_vector[hi++] = (uint16_t)( (sp->num_adj[target_vert[v]] >> 32) + 9173);
                                hash_vector[hi++] = (uint16_t)( (sp->num_adj[target_vert[v]] >> 48) + 9173);
#else
				return FALSE;
#endif
			}
		nc = (uint16_t) ( sp->partition[level+1].belongs[sp->partition[level+1].vertices[start+size-1]] + 1 );
	}
#ifdef RF
	if ( !compat )
                mismatch_found_hash = hash_Meiyan16 ( hash_vector, hi );
        } /* end of new scope */
#endif
	return compat;
}

uint8_t vertex_ref_compat ( SeqPart *sp, const struct graph *g, int level, uint16_t p, uint16_t *new_vertices, uint16_t *old_vertices, uint16_t *discarded_vert )
{
	uint16_t c; /* cells */
	uint16_t nc; /* number of valid cells */
	uint16_t counters[4]; /* number of vertices for each adjacency type */
	uint16_t positions[4]; /* */
	uint16_t v; /* the vertices in the cell being processed */
	int8_t adj;
	uint16_t nv; /* new vertices */
	uint8_t compat;
#ifdef RF
	uint16_t hash_size;
	uint16_t hi; /* hash index */

	for ( hash_size = 0, c = 0; c < sp->partition[level].noc; hash_size = (uint16_t)(hash_size + sp->partition[level].affected[c]), c++ );
	{ /* new scope for variables */
	uint16_t hash_vector[2+hash_size*12];
	hi = 0;
	hash_vector[hi++] = (uint16_t) level;
	hash_vector[hi++] = (uint16_t)(level >> 16);
	hi = (uint16_t)(hi - ( (level >> 16) == 0 ));
#endif

	compat = TRUE;
	sp->nodv = sp->partition[level].nopdv;
	discarded_vert[sp->nodv++] = p;
	nc = 0;
	nv = 0;
	for ( c = 0; c < sp->partition[level].noc; c++ )
	{
		if ( (uint16_t)sp->partition[level].attrib[c] == 0 )
		{
			ADD_DISCARDED_VERT( old_vertices, discarded_vert );
			continue;
		}
		if ( ! sp->partition[level].affected[c] )
		{
			for ( v = sp->partition[level].start[c]; v < sp->partition[level].start[c+1]; v++ )
				new_vertices[nv++] = old_vertices[v];
			nc++;
			continue;
		}
		COMPUTE_COUNTERS_AND_POSITIONS ( old_vertices );
		nv = (uint16_t) ( positions[NOT_ADJ] + counters[NOT_ADJ] );
		for ( adj = ARC_IO; adj >= NOT_ADJ; adj-- )
		{
			if ( counters[adj] == 0 )
				continue;
			if ( sp->partition[level+1].attrib[nc] != ( sp->partition[level].attrib[c] - adjacency[adj] ) || ( sp->partition[level+1].start[++nc] != ( positions[adj] + counters[adj] ) ) )
			{
#ifdef RF
				compat = FALSE;
				hash_vector[hi++] = nc;
				hash_vector[hi++] = counters[adj];
				hash_vector[hi++] = (uint16_t)(sp->partition[level].attrib[c] - adjacency[adj]);
#else
				return FALSE;
#endif
			}
		}
		for ( v = sp->partition[level].start[c]; v < sp->partition[level].start[c+1]; v++ )
			if ( p != old_vertices[v] )
				new_vertices[positions[g->adj[p][old_vertices[v]]]++] = old_vertices[v];
	}
#ifdef RF
	if ( !compat )
		mismatch_found_hash = hash_Meiyan16 ( hash_vector, hi );
	} /* end of new scope */
#endif
	return compat;
}

void merge_no_link_orbits_with_valid ( uint32_t level, SeqPart *sp, Graph_Orbits *go, uint16_t *vertices, uint8_t *valid )
{
	uint16_t c;	/* cell */
	uint16_t start;
	uint16_t v;

	for ( c = 0; c < sp->partition[level].nodc; c++ )
	{
		start = sp->partition[level].start[sp->partition[level].discarded_cells[c]];
		for ( v = start; v < sp->partition[level].start[sp->partition[level].discarded_cells[c]+1]; v++ )
		{
			valid[go->vertices[sp->partition[level].vertices[start]].belongs_to] = valid[go->vertices[sp->partition[level].vertices[start]].belongs_to] && valid[go->vertices[vertices[v]].belongs_to];
			valid[go->vertices[vertices[v]].belongs_to] = valid[go->vertices[sp->partition[level].vertices[start]].belongs_to];
			merge_orbits ( go, sp->partition[level].vertices[start], vertices[v] );
	
		}
	}
}

uint8_t last_part_compat ( Partition *p, uint16_t *vertices, const struct graph *g, const struct graph *h )
{
	uint16_t v1, v2;
	int8_t adj_old, adj_new;

	for ( v1 = 0; v1 < p->start[p->noc]; v1++ )
		for ( v2 = (uint16_t) ( v1 + 1 ); v2 < p->start[p->noc]; v2++ )
		{
			adj_old = g->adj[p->vertices[v1]][p->vertices[v2]];
			adj_new = h->adj[vertices[v1]][vertices[v2]];
			if ( adj_old != adj_new )
			{
#ifdef RF
				uint16_t mismatch_elements[3];

				mismatch_elements[0] = v1;
				mismatch_elements[1] = v2;
				mismatch_elements[2] = (uint16_t)(adj_new + 9173 );	/* 9173 is prime */
				mismatch_found_hash = hash_Meiyan16 ( mismatch_elements, 3 );
#endif
				return FALSE;
			}
		}
	return TRUE;
}

/*
void set_mr_partitions ( int level, SeqPart *sp, uint64_t *attrib[2], uint64_t *adeg[2], uint16_t **vertices, uint16_t **belongs, uint16_t **start, uint16_t **discarded_cells, uint8_t **affected )
{
	uint16_t i;

	for ( i = 0; i < 2; i++ )
	{
		sp->partition[level+i+2].attrib = attrib[i];
		sp->partition[level+i+2].adeg = adeg[i];
		sp->partition[level+i+2].vertices = vertices[i];
		sp->partition[level+i+2].belongs = belongs[i];
		sp->partition[level+i+2].start = start[i];
		sp->partition[level+i+2].discarded_cells = discarded_cells[i];
		sp->partition[level+i+2].affected = affected[i];
	}
}
*/

int multirefinement ( int level, SeqPart *sp, const struct graph *g )
{
	uint64_t attrib[2][g->num_vert];
	uint64_t adeg[2][g->num_vert];
	uint16_t vertices[2][g->num_vert];
	uint16_t belongs[2][g->num_vert];
/*	uint16_t start[2][g->num_vert+1]; */
	uint16_t discarded_cells[2][g->num_vert];
	uint8_t affected[2][g->num_vert];
	int reached_level;
	uint16_t nopdv;
	uint16_t pc;
	uint16_t bck;
	uint16_t i;
	uint8_t success;

	for ( i = 0; i < 2; i++ )
	{
		sp->partition[level+i+2].attrib = attrib[i];
		sp->partition[level+i+2].adeg = adeg[i];
		sp->partition[level+i+2].vertices = vertices[i];
		sp->partition[level+i+2].belongs = belongs[i];
/*		sp->partition[level+i+2].start = start[i]; */
		sp->partition[level+i+2].discarded_cells = discarded_cells[i];
		sp->partition[level+i+2].affected = affected[i];
	}
	nopdv = sp->nodv;
/*	set_mr_partitions ( level, sp, attrib, adeg, vertices, belongs, start, discarded_cells, affected ); */
	/* Allocated partition are backed up within level+3 partition */
	exchange_partitions ( &(sp->partition[level+1]), &(sp->partition[level+3]) );
	reached_level = level;
	vertex_refinement ( sp, g, level );
	sp->partition[level].tor = UNKNOWN;
	EXCHANGE( sp->valid_old, sp->valid_new, uint8_t* );
	reached_level++;
	pc = sp->partition[level+1].pivot = best_pivot ( g, sp, level+1, FALSE );
	success = FALSE;
	/* generating seqpart until equitable partition */
	while ( sp->valid_old[pc] && ( sp->partition[level+1].start[sp->partition[level+1].noc] > sp->partition[level+1].noc ) )
	{
/*		allocate_partition ( sp, level+1, (uint16_t)(sp->partition[level].noc), (uint16_t)(sp->partition[level].start[sp->partition[level].noc]) ); */
		/* VERTEX */
		if ( ( sp->partition[level+1].start[pc+1] - sp->partition[level+1].start[pc] ) == 1 )
		{
			vertex_refinement ( sp, g, level+1 );
			sp->partition[level+1].tor = VERTEX;
			success = TRUE;
		}
		/* SET */
		else
		{
			bck = sp->nodv;
			sp->valid_old[pc] = FALSE;
			set_refinement ( sp, g, level+1, &success );
			if ( ! success )
				sp->nodv = bck;
			else
				sp->partition[level+1].tor = SET;
		}
		if ( success )
		{
			EXCHANGE( sp->valid_old, sp->valid_new, uint8_t* );
			reached_level++;
			success = FALSE;
			exchange_partitions ( &(sp->partition[level+1]), &(sp->partition[level+2]) );
		}
		pc = sp->partition[level+1].pivot = best_pivot ( g, sp, level+1, FALSE );
	}
	sp->nodv = nopdv;
	/* Allocated partition are restored */
	exchange_partitions ( &(sp->partition[level+1]), &(sp->partition[level+3]) );
	for ( i = 0; i < sp->partition[level].noc; sp->valid_old[i++] = FALSE );
	return reached_level;
}

