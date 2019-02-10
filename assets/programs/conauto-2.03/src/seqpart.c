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

/*data structures and function to generate sequences of partitions */

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

#include "refinements.h"
#include "seqpart.h"
#include "sort.h"
#include "groups.h"
#include "graph.h"

Partition *p_global; /* due to compute cell types within 'qsort' routine */

int cmp_cell_size ( const void *a, const void *b )
{
	const uint16_t *c1 = (uint16_t*)a;
	const uint16_t *c2 = (uint16_t*)b;

	return (p_global->start[*c1 + 1] - p_global->start[*c1]) - (p_global->start[*c2 + 1] - p_global->start[*c2]);
}

uint16_t compute_cell_types ( int level, SeqPart *sp, uint16_t *valid_cells )
{
	uint64_t vct[sp->partition[level].noc];	/* valid cell types */
	uint8_t valid[sp->partition[level].noc];	/* valid cell types */
	uint16_t c, cc;
	uint16_t nof_valid;

	nof_valid = 0;
/*
	for ( c = 0; c < sp->partition[level].noc; c++ )
		valid[c] = ( sp->partition[level].attrib[c] != 0 );
	for ( c = 0; c < sp->partition[level].noc; c++ )
		vct[c] = ( ( ( sp->partition[level].attrib[c] ) & UINT64_C(0xffffffffffff0000) ) | ( (uint64_t)(sp->partition[level].start[c+1] - sp->partition[level].start[c]) & UINT64_C(0x000000000000ffff) ) );
	for ( c = 0; c < sp->partition[level].noc; c++ )
		if ( valid[c] )
			for ( cc = (uint16_t)(c+1); cc < sp->partition[level].noc; cc++ )
				if ( vct[c] == vct[cc] )
					valid[cc] = FALSE;
	for ( c = 0; c < sp->partition[level].noc; c++ )
		if ( valid[c] )
			valid_cells[nof_valid++] = c;
*/
	for ( c = 0; c < sp->partition[level].noc; c++ )
	{
		valid[c] = ( sp->partition[level].attrib[c] != 0 );
		vct[c] = ( ( ( sp->partition[level].attrib[c] ) & UINT64_C(0xffffffffffff0000) ) | ( (uint64_t)(sp->partition[level].start[c+1] - sp->partition[level].start[c]) & UINT64_C(0x000000000000ffff) ) );
	}
	for ( c = 0; c < sp->partition[level].noc; c++ )
		if ( valid[c] )
		{
			for ( cc = (uint16_t)(c+1); cc < sp->partition[level].noc; cc++ )
				if ( vct[c] == vct[cc] )
					valid[cc] = FALSE;
			valid_cells[nof_valid++] = c;
		}
/**/
	p_global = &(sp->partition[level]);
	qsort(valid_cells, nof_valid, sizeof ( uint16_t ), cmp_cell_size );			
	return nof_valid;
}

uint16_t best_individualized_cell ( const struct graph *g, SeqPart *sp, int level )
{
	uint16_t start[2][g->num_vert+1];
	uint16_t discarded_cells[g->num_vert];
	uint8_t affected[g->num_vert];
	uint16_t vc[sp->partition[level].noc];	/* valid cells */
	uint16_t nof_valid;	/* number of valid cells */
	int reached_level;
	uint16_t c, best_cell;
	uint16_t best_noc;	/* maximum number of cells */
	uint16_t best_size;	/* minimum pivot cell size */
	uint16_t i;

/* INFO:
	level:		to refine partition level
	level+1:	new partition level (maybe allocated preoviusly)
	level+2:	virtual (reached level - 1) partition
	level+3:	virtual reached level partition
*/
	nof_valid = compute_cell_types ( level, sp, vc );
	best_cell = 0;
	best_noc = 0;
	best_size = UINT16_MAX;
	sp->partition[level+2].start = start[0];
	sp->partition[level+3].start = start[1];
	/* if new level has not been allocated yet */
	if ( sp->partition[level+1].vertices == NULL )
	{
		sp->partition[level].discarded_cells = discarded_cells;
		sp->partition[level].affected = affected;
	}
	for ( i = 0; i < nof_valid; i++ )
	{
		sp->dcs_nodes++;
		sp->partition[level].pivot = c = vc[i];
		reached_level = multirefinement ( level, sp, g );
		if ( sp->partition[level+3].start[sp->partition[level+3].noc] == sp->partition[level+3].noc )
		{
			best_cell = c;
			break;
		}
		if ( is_subpartition( sp, level+3, level ) )
		{
			best_cell = c;
			break;
		}
		/* if split more cells */
		if ( sp->partition[level+3].noc + ( sp->partition[level].start[sp->partition[level].noc] - sp->partition[level+3].start[sp->partition[level+3].noc] ) > best_noc )
		{
			best_noc = (uint16_t)( sp->partition[level+3].noc + (uint16_t)( sp->partition[level].start[sp->partition[level].noc] - sp->partition[level+3].start[sp->partition[level+3].noc] ) );
			best_cell = c;
			best_size = (uint16_t)(sp->partition[level].start[sp->partition[level].pivot+1] - sp->partition[level].start[sp->partition[level].pivot] );
		}
		else
			/* if split the same cells, but initial pivot cell is smaller */
			if ( ( ( sp->partition[level+3].noc + ( sp->partition[level].start[sp->partition[level].noc] - sp->partition[level+3].start[sp->partition[level+3].noc] ) ) == best_noc ) && ( best_size > (uint16_t)(sp->partition[level].start[sp->partition[level].pivot+1] - sp->partition[level].start[sp->partition[level].pivot] ) ) )
			{
				best_cell = c;
				best_size = (uint16_t)(sp->partition[level].start[sp->partition[level].pivot+1] - sp->partition[level].start[sp->partition[level].pivot] );
			}
	}
	sp->partition[level+2].vertices = NULL;
	sp->partition[level+3].vertices = NULL;
	return best_cell;
}

uint16_t best_pivot ( const struct graph *g, SeqPart *sp, int level, uint8_t choose_not_valid )
{
	uint16_t best;
	uint16_t i;
	uint16_t best_size;
	uint16_t best_attrib;
	Partition *p = &(sp->partition[level]);

	best = 0;
	best_size = p->start[1];
	best_attrib = (uint16_t)p->attrib[0];
	for ( i = 1; i < p->noc; i++ )
		if ( sp->valid_old[i] )
			if ( !sp->valid_old[best] )
			{
				best = i;
				best_size = (uint16_t) ( p->start[i+1] - p->start[i] );
				best_attrib = (uint16_t)p->attrib[i];
			}
			else
				if ( best_size > ( p->start[i+1] - p->start[i] ) )
				{
					best = i;
					best_size = (uint16_t) ( p->start[i+1] - p->start[i] );
					best_attrib = (uint16_t)p->attrib[i];
				}
				else
					if ( ( best_size == ( p->start[i+1] - p->start[i] ) ) && ( best_attrib < (uint16_t)p->attrib[i] ) )
					{
						best = i;
						best_attrib = (uint16_t)p->attrib[i];
					}
					else;
#ifdef PCS
		else;
	/* choosing best pivot cell between not valid cells */
	if ( choose_not_valid && !sp->valid_old[best] && sp->partition[level].noc > 1 )
		return best_individualized_cell ( g, sp, level );
#else
		else
			if ( !sp->valid_old[best] && (uint16_t)p->attrib[i] )
				if ( best_attrib == 0 || best_size > (p->start[i+1] - p->start[i]) )
				{
					best = i;
					best_size = (uint16_t)(p->start[i+1] - p->start[i]);
					best_attrib = (uint16_t)(p->attrib[i]);
				}
#endif
	return ( best );
}

void mark_cells_with_no_links ( Partition *p )
{
	uint16_t c; /* cell */
	
	p->nodc = 0;
	for ( c = 0; c < p->noc; c++ )
		if ( (uint16_t)p->attrib[c] == 0 )
			p->discarded_cells[p->nodc++] = c;
}

void free_partitions ( SeqPart *sp, int level )
{
	int cl;	/* current level */
	Partition *cp; /* current partition */
	Partition *pp; /* previous partition */

	cp = NULL;
	for ( cl = level; sp->partition[cl].vertices != NULL; cl++ )
	{
		cp = &(sp->partition[cl]);
		pp = &(sp->partition[cl-1]);
		free ( pp->affected );
		free ( pp->discarded_cells );
		free ( cp->vertices );
		free ( cp->belongs );
		free ( cp->start );
		free ( cp->attrib );
		if ( cp->adeg != NULL )
			free ( cp->adeg );
		cp->vertices = NULL;
		if ( cp->mm_collection != NULL )
		{
			free ( cp->mm_collection->mismatch_hash );
			free ( cp->mm_collection->counters );
			free ( cp->mm_collection );
			cp->mm_collection = NULL;
		}
		if ( cp->go != NULL )
		{
			free ( cp->go->vertices );
			free ( cp->go->orbits );
			free ( cp->go );
			cp->go = NULL;
		}
	}
	free ( cp->discarded_cells ); /* warning: if do not free partitions up to LAST */
}

void allocate_partition ( SeqPart *sp, int level, uint16_t noc, uint16_t num_vert )
{
	Partition *cp; /* current partition */
	Partition *pp; /* previous partition */

	cp = &(sp->partition[level]);
	pp = &(sp->partition[level-1]);
	if ( cp->vertices == NULL )
	{
		pp->affected = malloc ( noc * sizeof ( uint8_t ) );
		pp->discarded_cells = malloc ( noc * sizeof ( uint16_t ) );
		cp->vertices = malloc ( num_vert * sizeof ( uint16_t ) );
		cp->belongs = malloc ( sp->partition[0].start[sp->partition[0].noc] * sizeof ( uint16_t ) );
		cp->start = calloc ( (size_t)( num_vert + 1 ), sizeof ( uint16_t ) );
		cp->attrib = malloc ( num_vert * sizeof ( uint64_t ) );
		cp->adeg = malloc ( num_vert * sizeof ( uint64_t ) );
	}
}

void free_seqpart ( SeqPart *sp )
{
	free_partitions ( sp, 1 );
	free ( sp->partition[0].vertices );
	free ( sp->partition[0].belongs );
	free ( sp->partition[0].start );
	free ( sp->partition[0].attrib );
	if ( sp->partition[0].adeg != NULL )
		free ( sp->partition[0].adeg );
	sp->partition[0].vertices = NULL;
	if ( sp->partition[0].mm_collection != NULL )
	{
		free ( sp->partition[0].mm_collection->mismatch_hash );
		free ( sp->partition[0].mm_collection->counters );
		free ( sp->partition[0].mm_collection );
		sp->partition[0].mm_collection = NULL;
	}
	if ( sp->partition[0].go != NULL )
	{
		free ( sp->partition[0].go->vertices );
		free ( sp->partition[0].go->orbits );
		free ( sp->partition[0].go );
		sp->partition[0].go = NULL;
	}
	free ( sp->partition );
	free ( sp->discarded_vert );
	free ( sp->valid_new );
	free ( sp->valid_old );
	free ( sp->num_adj );
}

void allocate_seqpart ( SeqPart *sp, uint32_t size )
{
	Partition *cp;

	sp->partition = calloc ( (size_t)( size * 2 ), sizeof ( Partition ) );
	sp->nodv = 0;
	sp->discarded_vert = malloc ( size * sizeof ( uint16_t ) );
	sp->valid_new = malloc ( size * sizeof ( uint8_t ) );
	sp->valid_old = malloc ( size * sizeof ( uint8_t ) );
	sp->num_adj = malloc ( size * sizeof ( uint64_t ) );
	sp->nodes = 1;
	sp->nodes_limit = UINT32_MAX;
	sp->dcs_nodes = 0;
	sp->bad_nodes = 0;
	cp = &(sp->partition[0]);
	cp->vertices = malloc ( size * sizeof ( uint16_t ) );
	cp->belongs = malloc ( size * sizeof ( uint16_t ) );
	cp->start = calloc ( (size_t)( size + 1 ), sizeof ( uint16_t ) );
	cp->attrib = malloc ( size * sizeof ( uint64_t ) );
	cp->adeg = malloc ( size * sizeof ( uint64_t ) );
}

void generate_degree_partition ( SeqPart *sp, const struct graph *g )
{
	uint16_t v;
	uint64_t adeg; /* current degree of the cell processed */
	uint16_t noc; /* number of cells */
	Partition *p = &(sp->partition[0]);

	sp->partition[0].nopdv = 0;
	for ( v = 0; v < g->num_vert; v++ )
		p->vertices[v] = v;
	sort_cell ( g->num_vert, p->vertices, g->deg );
	p->start[0] = 0;
	p->belongs[p->vertices[0]] = 0;
	adeg = g->deg[p->vertices[0]];
	p->attrib[0] = adeg;
	sp->valid_old[0] = ( adeg > 0 );
	noc = 1;
	for ( v = 1; v < g->num_vert; v++ )
	{
		if ( g->deg[p->vertices[v]] != adeg )
		{
			p->start[noc] = v;
			adeg = g->deg[p->vertices[v]];
			p->attrib[noc] = adeg;
			sp->valid_old[noc] = ( adeg > 0 );
			noc++;
		}
		p->belongs[p->vertices[v]] = (uint16_t) ( noc - 1 );
	}
	p->start[noc] = v;
	sp->valid_old[0] = sp->valid_old[0] && ( noc > 1 );
	p->noc = noc;
}

void exchange_partitions ( Partition *p1, Partition *p2 )
{
	EXCHANGE( p1->attrib, p2->attrib, uint64_t* );
	EXCHANGE( p1->adeg, p2->adeg, uint64_t* );
	EXCHANGE( p1->vertices, p2->vertices, uint16_t* );
	EXCHANGE( p1->belongs, p2->belongs, uint16_t* );
	EXCHANGE( p1->start, p2->start, uint16_t* );
	EXCHANGE( p1->discarded_cells, p2->discarded_cells, uint16_t* );
	EXCHANGE( p1->affected, p2->affected, uint8_t* );
	EXCHANGE( p1->pivot, p2->pivot, uint16_t );
	EXCHANGE( p1->noc, p2->noc, uint16_t );
	EXCHANGE( p1->nodc, p2->nodc, uint16_t );
	EXCHANGE( p1->nopdv, p2->nopdv, uint16_t );
	EXCHANGE( p1->tor, p2->tor, uint8_t );
}

void generate_seqpart ( SeqPart *sp, const struct graph *g, int level )
{
	uint16_t pc; /* pivot cell */
	uint8_t success;
	uint16_t bck;

	while ( sp->partition[level].start[sp->partition[level].noc] > sp->partition[level].noc )
	{

		pc = sp->partition[level].pivot = best_pivot ( g, sp, level, TRUE );

		if ( (uint16_t)sp->partition[level].attrib[pc] == 0 )
			break;
		level++;
		allocate_partition ( sp, level, sp->partition[level-1].noc, sp->partition[level-1].start[sp->partition[level-1].noc] );
		/* VERTEX */
		if ( ( sp->partition[level-1].start[pc+1] - sp->partition[level-1].start[pc] ) == 1 )
		{
			vertex_refinement ( sp, g, level-1 );
			sp->partition[level-1].tor = VERTEX;
			free ( sp->partition[level].adeg );
			sp->partition[level].adeg = NULL;
		}
		/* SET */
		else if ( sp->valid_old[pc] ) /* SET */
		{
			sp->valid_old[pc] = FALSE;
			bck = sp->nodv;
			set_refinement ( sp, g, level-1, &success );
			sp->partition[level-1].tor = SET;
			if ( ! success )
			{
				level--;
				sp->nodv = bck;
				continue;
			}
		}
		/* UNKNOWN: individualization */
		else
		{
			vertex_refinement ( sp, g, level-1 );
			sp->partition[level-1].tor = UNKNOWN;
			free ( sp->partition[level].adeg );
			sp->partition[level].adeg = NULL;
			sp->nodes++;
		}
		EXCHANGE( sp->valid_old, sp->valid_new, uint8_t* );
	}
	sp->partition[level].tor = LAST;
	sp->partition[level+1].start = NULL;
	sp->partition[level].discarded_cells = malloc ( sp->partition[level].noc * sizeof ( uint16_t ) );
	mark_cells_with_no_links ( &(sp->partition[level]) );
	sp->lp = level;
	memcpy ( sp->discarded_vert + sp->nodv, sp->partition[level].vertices, sp->partition[level].start[sp->partition[level].noc] * sizeof (uint16_t ) );
}

int degree_partitions_are_compatible ( Partition *p1, Partition *p2 )
{
	int compatible;
	uint16_t c;

	for ( compatible = ( p1->noc == p2->noc ), c = 0; compatible && c < p1->noc; c++ )
		compatible = ( p1->start[c] == p2->start[c] && p1->attrib[c] == p2->attrib[c] );
	return compatible;
}

/* int is_not_worth ( SeqPart *sp, int bl, int level ) */
int is_subpartition ( SeqPart *sp, int level, int bl )
{
	const Partition *p = &(sp->partition[level]);
	uint16_t c1, c2;

	c1 = 0;
	c2 = 1;
	while ( c2 < p->noc )
	{
		if ( (uint16_t)p->attrib[c1] == 0 )
		{
			c1++;
			c2++;
		}
		else if ( (uint16_t)p->attrib[c2] == 0 )
			c2++;
		else if ( sp->partition[bl].belongs[p->vertices[p->start[c1]]] == sp->partition[bl].belongs[p->vertices[p->start[c2]]] )
			return FALSE;
		else
		{
			c1 = c2;
			c2++;
		}
	}
	return TRUE;
}

void compute_auto_search_limit ( SeqPart *sp, int level )
{
	int target_level;

	target_level = level+1;
	while ( target_level < sp->lp && ( sp->partition[target_level].tor != UNKNOWN || !is_subpartition ( sp, target_level, level ) ) )
		target_level++;
	sp->partition[level].auto_search_limit = target_level;
}

void compute_auto_search_limits ( SeqPart *sp )
{
	int level;

	for ( level = 0; level < sp->lp; level++ )
		if ( sp->partition[level].tor == UNKNOWN )
			compute_auto_search_limit ( sp, level );
}

void compute_backtrack_levels ( SeqPart *sp )
{
	int level;
	int bl; /* backtrack level */

	for ( level = sp->lp; level >= 0; level-- )
#ifdef BKJ
		if ( sp->partition[level].tor == UNKNOWN )
		{
			for ( bl = level - 1; bl >= 0 && ( sp->partition[bl].tor != UNKNOWN || is_subpartition ( sp, level, bl ) ); bl-- );
			sp->partition[level].backtrack_level = bl;
		}
		else
#endif
			sp->partition[level].backtrack_level = level-1;
}

uint16_t num_backtrack_levels ( SeqPart *sp )
{
	uint16_t count;
	uint16_t level;

	for ( count = 0, level = 0; level < sp->lp; level++ )
		count = (uint16_t) ( count + ( sp->partition[level].tor == BACKTR ) );
	return count;
}

void process_cells_with_no_links ( Partition *p, Perm_Group *pg )
{
	uint16_t c; /* cell */
	uint16_t v;
	uint16_t previous;
	uint16_t perm[pg->nelem];
	uint16_t cell_size;

	memcpy ( perm, pg->base, pg->nelem * sizeof ( uint16_t ) );
	for ( c = 0, previous = (uint16_t) ( p->nopdv + ( p->tor == VERTEX || p->tor == UNKNOWN ) ); c < p->nodc; c++, previous = (uint16_t) (previous + cell_size ) )
	{
		cell_size = (uint16_t) ( p->start[p->discarded_cells[c]+1] - p->start[p->discarded_cells[c]] );
		for ( v = 1; v < cell_size; v++ )
		{
			perm[previous] = p->vertices[p->start[p->discarded_cells[c]] + v];
			perm[previous + v] = p->vertices[p->start[p->discarded_cells[c]]];
			set_perm ( pg, perm );
			merge_orbits ( &(pg->orbits), perm[previous], perm[previous + v] );
			perm[previous + v] = p->vertices[p->start[p->discarded_cells[c]] + v];
			perm[previous] = p->vertices[p->start[p->discarded_cells[c]]];
		}
	}
}

void check_cells_with_no_links ( Partition *p, Perm_Group *pg, uint16_t *vertices_old )
{
	uint16_t c; /* cell */
	uint16_t u, v, w;
	uint16_t perm[pg->nelem];

	memcpy ( perm, pg->base, pg->nelem * sizeof ( uint16_t ) );
	for ( c = 0; c < p->nodc; c++ )
	{
		u = pg->inv_base[vertices_old[p->start[p->discarded_cells[c]]]];
		for ( v = (uint16_t) ( p->start[p->discarded_cells[c]]+1 ); v <  p->start[p->discarded_cells[c]+1]; v++ )
		{
			w = pg->inv_base[vertices_old[v]];
			perm[u] = vertices_old[v];
			perm[w] = vertices_old[p->start[p->discarded_cells[c]]];
			if ( !perm_in_pg ( pg, perm ) )
			{
				set_perm ( pg, perm );
				merge_orbits ( &(pg->orbits), perm[u], perm[w] );
			}
			perm[w] = vertices_old[v];
			perm[u] = vertices_old[p->start[p->discarded_cells[c]]];
		}
	}
}

uint8_t equal_partitions ( int level, uint16_t *vertices, SeqPart *sp )
{
	uint16_t c; /* cell being processed */
	uint16_t v; /* vertex in the cell */

	for ( c = 0; c < sp->partition[level].noc; c++ )
		if ( (uint16_t)sp->partition[level].attrib[c] )
			for ( v = sp->partition[level].start[c]; v < sp->partition[level].start[c+1]; v++ )
				if ( sp->partition[level].vertices[v] != vertices[v] )
					return FALSE;
	return TRUE;
}

void push_perm_to_discarded_vert ( Perm_Group *pg, Permutation perm, Partition *p, uint16_t *vertices, uint32_t num_vert )
{
	const Permutation base = pg->base;
	uint16_t c;
	uint16_t v;
	uint16_t i;
	uint16_t start;
	uint16_t cell_size;

	for ( i = p->nopdv; i < num_vert; i++ )
		perm[i] = base[i];
	for ( c = 0; c < p->nodc; c++ )
	{
		start = p->start[p->discarded_cells[c]];
		cell_size = (uint16_t) ( p->start[p->discarded_cells[c]+1] - start );
		for ( v = 0; v < cell_size; v++ )
			perm[pg->inv_base[p->vertices[start + v]]] = vertices[start + v];
	}
}

/* algorithm of sub-partitions theorem */
void process_contained_cells ( int level, uint16_t *vertices, SeqPart *sp, Perm_Group *pg, int backtrack_level )
{
	const Partition *p = &(sp->partition[level]);
	uint16_t previous;
	uint16_t cell_size;
	uint16_t c;	/* cell */
	uint16_t v, w;
	uint16_t offset;
	uint16_t inv_dv[pg->nelem];	/* discarded_vert inverse perm */

	memcpy ( ( sp->discarded_vert + sp->nodv ), ( pg->base + sp->nodv ), (size_t)( pg->nelem - sp->nodv ) * sizeof ( uint16_t ) );
	/* Push discarded vertices in discarded_vert, and compute number of these vertices (offset). */
	offset = 0;
	for ( c = 0, previous = (uint16_t)( p->nopdv + 1 ); c < p->nodc; c++, previous = (uint16_t) (previous + cell_size ) )
	{
		cell_size = (uint16_t) ( p->start[p->discarded_cells[c]+1] - p->start[p->discarded_cells[c]] );
		for ( v = 0; v < cell_size; v++ )
			sp->discarded_vert[previous+v] = vertices[p->start[p->discarded_cells[c]]+v];
		offset = (uint16_t)( offset + cell_size );
	}
	inv_perm_partialy ( sp->discarded_vert, inv_dv, pg->nelem, sp->partition[backtrack_level].nopdv, (uint16_t)(sp->nodv + offset + 1), sp->discarded_vert[sp->nodv] );
	/* fill in new perm */
	for ( v = sp->partition[backtrack_level].nopdv; v < ( sp->partition[level].nopdv + offset + 1 ); v++ )
		if ( pg->inv_base[sp->discarded_vert[v]] >= (sp->partition[level].nopdv + offset +1) || pg->inv_base[sp->discarded_vert[v]] == sp->partition[level].nopdv )
		{
			w =  pg->base[v];
			while (inv_dv[w] != UINT16_MAX) {
				w = pg->base[inv_dv[w]];
			};
			sp->discarded_vert[pg->inv_base[sp->discarded_vert[v]]] = w;
		}
}

uint8_t equal_partial_orbits ( int level, SeqPart *sp, Graph_Orbits *ogo, Graph_Orbits *pgo )
{
	const uint16_t pc = sp->partition[level].pivot; /* pivot cell */
	const size_t pc_size = (size_t)(sp->partition[level].start[pc+1] - sp->partition[level].start[pc]); /* pivot cell size */
	uint16_t oo_size_count[pc_size-1];	/* number of original orbits of each size */
	uint16_t po_size_count[pc_size-1];	/* number of partial orbit of each size */
	uint8_t oo_processed[sp->partition[0].start[sp->partition[0].noc]];
	uint8_t po_processed[pc_size];
	uint16_t v;

	for ( v = 0; v < (pc_size-1); oo_size_count[v] = po_size_count[v] = 0, v++ );
	for ( v = 0; v < pc_size; po_processed[v++] = FALSE );
	for ( v = 0; v < sp->partition[0].start[sp->partition[0].noc]; oo_processed[v++] = FALSE );
	for ( v = 0; v < pc_size; v++ )
		if ( ! oo_processed[ogo->vertices[sp->partition[level].vertices[sp->partition[level].start[pc] + v]].belongs_to] )
		{
			oo_size_count[ogo->orbits[ogo->vertices[sp->partition[level].vertices[sp->partition[level].start[pc] + v]].belongs_to].size-1]++;
			oo_processed[ogo->vertices[sp->partition[level].vertices[sp->partition[level].start[pc] + v]].belongs_to] = TRUE;
		}
	for ( v = 0; v < pc_size; v++ )
		if ( ! po_processed[pgo->vertices[v].belongs_to] )
		{
			po_size_count[pgo->orbits[pgo->vertices[v].belongs_to].size-1]++;
			po_processed[pgo->vertices[v].belongs_to] = TRUE;
		}
	return !memcmp ( oo_size_count, po_size_count, (pc_size-1) * sizeof ( uint16_t ) );
}

void compute_orbits_order ( int level, SeqPart *sp, Graph_Orbits *go, uint16_t *vertices, uint16_t *valid_vert_index )
{
	const uint16_t pc = sp->partition[level].pivot; /* pivot cell */
	const uint16_t pc_size = (uint16_t) (sp->partition[level].start[pc+1] - sp->partition[level].start[pc]); /* pivot cell size */
	uint16_t i, j;
	uint16_t valid_vertices[pc_size-1];

	for ( i = 1; i < pc_size; i++ )
		valid_vertices[(uint16_t)(i-1)] = vertices[(uint16_t)(sp->partition[level].start[pc]+i)];
	sort_valid_orbits ( (uint16_t)(pc_size-1), valid_vertices, go );
	for ( i = 0; i < (uint16_t)(pc_size-1); i++ )
	{
		for ( j = 1; j < pc_size; j++ )
			if ( valid_vertices[i] == vertices[(uint16_t)(sp->partition[level].start[pc]+j)] )
			{
				valid_vert_index[(uint16_t)(pc_size-2-i)] = j;
				break;
			}
	}
}

uint16_t compute_partial_orbits_order ( int level, SeqPart *sp, Graph_Orbits *ogo, Graph_Orbits *pgo, /* uint16_t *vertices,*/ uint16_t *valid_vertices )
{
	const uint16_t pc = sp->partition[level].pivot; /* pivot cell */
	const uint16_t pc_size = (uint16_t) (sp->partition[level].start[pc+1] - sp->partition[level].start[pc]); /* pivot cell size */
	const uint16_t pc_orbitSize = ogo->orbits[ogo->vertices[sp->partition[level].vertices[sp->partition[level].start[pc]]].belongs_to].size;
	uint16_t nof_valid;
	uint16_t i;
	uint8_t po_compat; /*partial orbits compatible */

	nof_valid = 0;
	po_compat = equal_partial_orbits ( level, sp, ogo, pgo );
	for ( i = 0; i < pc_size; i++ )
		if ( po_compat )
			if (pgo->orbits[pgo->vertices[i].belongs_to].size == pc_orbitSize )
				valid_vertices[nof_valid++] = i;
			else;
		else if ( pgo->orbits[pgo->vertices[i].belongs_to].size <= pc_orbitSize )
		{
			valid_vertices[nof_valid++] = i;
		}
/*
	if ( !po_compat )
		sort_valid_orbits ( nof_valid, valid_vertices, pgo );
*/
	return nof_valid;
}

void generate_partial_orbits ( SeqPart *sp, int level, uint16_t* vertices_old, Perm_Group *pg, Graph_Orbits *pgo, int prev_bl, uint16_t *discarded_vert, uint16_t *old_perm_sets, uint16_t *old_leaders, uint16_t *new_perm_sets, uint16_t *new_leaders )
{
	const uint16_t pc = sp->partition[level].pivot; /* pivot cell */
	uint16_t inv [pg->nelem];
	uint16_t i, j;
	uint16_t p1, p2;
	
	memcpy ( new_perm_sets, old_perm_sets, pg->nelem * sizeof ( uint16_t ) );
	memcpy ( new_leaders, old_leaders, pg->nelem * sizeof ( uint16_t ) );
	create_trivial_orbits ( pgo );
	for ( i = 0; i < pg->nelem; inv[i++] = UINT16_MAX );
	for ( i = 0; i < pgo->size; inv[vertices_old[sp->partition[level].start[pc]+i]] = i, i++ );
	for ( p1 = 0; p1 < pg->nperms; p1++ )
		if ( new_leaders[p1] )
		{
			uint16_t prev_perm_not_valid;	/* previous permutation not valid */
			uint16_t prev_perm_valid;	/* previous permutation valid */

			prev_perm_not_valid = UINT16_MAX;
			prev_perm_valid = p1;
			p2 = new_perm_sets[p1];
			while ( p2 != UINT16_MAX )
			{
				if ( fixed_in_2_perms ( pg, p1, p2, discarded_vert, sp->partition[prev_bl].nopdv, sp->partition[level].nopdv ) )
				{
					/* merge orbits respect both (p1 and p2) perms */
					prev_perm_valid = p2;
					for ( i = sp->partition[level].start[pc]; i < sp->partition[level].start[pc+1]; i++ )
						if ( ( j = inv[pg->perms[p2][pg->inv_perms[p1][vertices_old[i]]]] ) != UINT16_MAX )
							merge_orbits ( pgo, (uint16_t)(i - sp->partition[level].start[pc]), j );
				}
				else
				{
					/* split permutation set */
					if ( prev_perm_not_valid == UINT16_MAX )
					{
						prev_perm_not_valid = p2;
						new_leaders[p2] = TRUE;
					}
					else
					{
						new_perm_sets[prev_perm_not_valid] = p2;
						prev_perm_not_valid = p2;
					}
					new_perm_sets[prev_perm_valid] = new_perm_sets[p2];
				}				
				p2 = new_perm_sets[p2];
			}
			if ( prev_perm_not_valid != UINT16_MAX )
				new_perm_sets[prev_perm_not_valid] = UINT16_MAX;
			if ( new_perm_sets[p1] == UINT16_MAX )
				new_leaders[p1] = FALSE;
		}
}

#define CHECK_VALID( vertices, orbits ) \
{	uint16_t i; \
\
	for ( i = 0; i < w && !are_in_the_same_orbit ( valid_vertices[i], valid_vertices[w], orbits ); i++ ); \
	valid = ( i == w );\
}

int subtree_compat ( int level, uint16_t *vertices_old, const struct graph *g, SeqPart *sp, Perm_Group *pg, uint16_t *perm_sets, uint16_t *leaders, int init_bl, int prev_bl );

int deepen_in_backtrack ( int level, uint16_t *vertices_new, uint16_t *vertices_old, const struct graph *g, SeqPart *sp, Perm_Group *pg, uint16_t *old_perm_sets, uint16_t *old_leaders, int init_bl, int prev_bl )
{
	const uint16_t pc = sp->partition[level].pivot; /* pivot cell */
	const uint16_t pc_size = (uint16_t) (sp->partition[level].start[pc+1] - sp->partition[level].start[pc]); /* pivot cell size */
	uint16_t new_perm_sets[pg->nelem];
	uint16_t new_leaders[pg->nelem];
	int backtrack_level;
	uint16_t v, w;
	uint8_t valid;
	Info_Vertex info_vertex[pc_size];
	Info_Orbit info_orbit[pc_size];
	Graph_Orbits pgo = { info_vertex, info_orbit, pc_size }; /* partial Graph_Orbits */
	uint16_t counters[pc_size];
	uint16_t valid_vertices[pc_size];
	uint16_t nof_valid;

	for ( v = 0; v < pc_size; counters[v++] = 0 );
#ifdef PGO
	generate_partial_orbits ( sp, level, vertices_old, pg, &pgo, prev_bl, sp->discarded_vert, old_perm_sets, old_leaders, new_perm_sets, new_leaders );
	nof_valid = compute_partial_orbits_order ( level, sp, sp->partition[level].go, &pgo, /* vertices_old,*/ valid_vertices );
	for ( w = 0; w < nof_valid; w++ )
	{
		v = (uint16_t)(sp->partition[level].start[pc] + valid_vertices[w]);
		CHECK_VALID( vertices_old, &pgo );
		if ( valid )
#else
	for( v = sp->partition[level].start[pc]; v < sp->partition[level].start[pc+1]; v++ )
	{
#endif
		{
			uint16_t i, n;

			sp->nodes++;
			if ( vertex_ref_compat ( sp, g, level, vertices_old[v], vertices_new, vertices_old, sp->discarded_vert ) )
			{
				if ( ( backtrack_level = subtree_compat ( level + 1, vertices_new, g, sp, pg, new_perm_sets, new_leaders, init_bl, level ) ) != level )
				{
					if( backtrack_level < level )
						sp->bad_nodes++;
					return backtrack_level;
				}
				else
					sp->bad_nodes++;
			}
			if( sp->nodes >= sp->nodes_limit )
			{
				printf( "Nodes limit:\t%"PRIu32"\nNodes:\t\t%"PRIu32"\n", sp->nodes_limit, sp->nodes );
				exit(150); /* NODES LIMIT EXCEEDED */
			}
#ifdef RF
#ifdef PGO
			for ( i = (uint16_t)(w+1), n = 1; i < nof_valid; i++ )
				if ( are_in_the_same_orbit ( valid_vertices[i], valid_vertices[w], &pgo ) )
					n++;
#else
			n = 1;
#endif
			if ( !mismatch_in_collection ( sp->partition[level].mm_collection, counters, n ) )
			{
				break;
			}
#endif
		}
	}
#ifdef RF
	mismatch_found_hash = UINT16_MAX;
#endif
	return sp->partition[level].backtrack_level;
}

int subtree_compat ( int level, uint16_t *vertices_old, const struct graph *g, SeqPart *sp, Perm_Group *pg, uint16_t *perm_sets, uint16_t *leaders, int init_bl, int prev_bl )
{
	uint16_t vertices_new [sp->partition[level].start[sp->partition[level].noc]];
	uint16_t p;	/* pivot vertex */
	uint16_t pc_size;	/* pivot cell size */
	int backtrack_level;

/*	check_cells_with_no_links ( &(sp->partition[level]), pg, vertices_old ); */
#ifdef EAD
	if ( equal_partitions ( level, vertices_old, sp ) )
	{
		push_perm_to_discarded_vert ( pg, sp->discarded_vert, &(sp->partition[level]), vertices_old, g->num_vert );
		return level;
	}
	if ( level == sp->partition[init_bl].auto_search_limit && sp->partition[level].tor != LAST )
	{
		process_contained_cells ( level, vertices_old, sp, pg, init_bl );
		return level;
	}
#endif
	switch ( sp->partition[level].tor )
	{
		case BACKTR:
#ifdef AGC
			return deepen_in_backtrack ( level, vertices_new, vertices_old, g, sp, pg, perm_sets, leaders, init_bl, prev_bl );
#else
			break;
#endif
		case LAST:
			if ( last_part_compat ( &(sp->partition[level]), vertices_old, g, g ) )
			{
				memcpy ( sp->discarded_vert + sp->nodv, vertices_old, sp->partition[level].start[sp->partition[level].noc] * sizeof ( uint16_t ) );
				return level;
			}
			break;
		case SET:
			if ( set_ref_compat ( sp, g, level, vertices_new, vertices_old, sp->discarded_vert ) )
				if ( ( backtrack_level = subtree_compat ( level + 1, vertices_new, g, sp, pg, perm_sets, leaders, init_bl, prev_bl ) ) != level )
					return backtrack_level;
			break;
		case VERTEX:
		default:
			p = vertices_old[sp->partition[level].start[sp->partition[level].pivot]];
			pc_size = sp->partition[level].start[sp->partition[level].pivot+1] - sp->partition[level].start[sp->partition[level].pivot];
			if ( pc_size > 1 )
				sp->nodes++;
			if ( vertex_ref_compat ( sp, g, level, p, vertices_new, vertices_old, sp->discarded_vert ) )
				if ( ( backtrack_level = subtree_compat ( level + 1, vertices_new, g, sp, pg, perm_sets, leaders, init_bl, prev_bl ) ) != level )
				{
					if( (pc_size > 1) && (backtrack_level < level) )
						sp->bad_nodes++;
					return backtrack_level;
				}
				else
					if( pc_size > 1 )
						sp->bad_nodes++;
					else;
			else;
#ifdef RF
			if ( pc_size > 1 )
				mismatch_found_hash = UINT16_MAX;
#endif
			break;
	}
	return sp->partition[level].backtrack_level;
}

int generate_automorphism ( int level, uint16_t p, uint16_t *vertices_old, const struct graph *g, SeqPart *sp, Perm_Group *pg, uint16_t *perm_sets, uint16_t *leaders )
{
	uint16_t vertices_new[sp->partition[level].start[sp->partition[level].noc]];

	if ( vertex_ref_compat ( sp, g, level, p, vertices_new, vertices_old, sp->discarded_vert ) )
		return subtree_compat ( ( level + 1 ), vertices_new, g, sp, pg, perm_sets, leaders, level, level );
	return sp->partition[level].backtrack_level;
}

#define CHECK_IF_VALID \
{	uint16_t i; \
\
	for ( i = sp->partition[level].start[pc]; i < v && !are_in_the_same_orbit ( sp->partition[level].vertices[i], sp->partition[level].vertices[v], &(pg->orbits) ); i++ ); \
	valid = ( i == v ); \
}

uint8_t check_automorphism ( const struct graph *g, SeqPart *sp, Perm_Group *pg, int level )
{
	const uint16_t pc = sp->partition[level].pivot;	/* pivot cell */
	const uint16_t pc_size = (uint16_t)( sp->partition[level].start[pc+1] - sp->partition[level].start[pc] );
	const uint16_t pivot = sp->partition[level].start[pc];
	const Graph_Orbits *go = &(pg->orbits);
	uint16_t v;
	uint16_t i;
	uint8_t tor;	/* type of refinement */
	uint8_t valid;
	uint8_t count_orbits[g->num_vert];
	uint16_t perm_sets[g->num_vert];	/* permutation sets */
	uint16_t leaders[g->num_vert];	/* leader of each permutation set */
	uint16_t previous_nperms;
#ifdef RF
	uint8_t mismatch_marked[pc_size];

	for ( v = 0; v < pc_size; mismatch_marked[v++] = FALSE );
#endif
	previous_nperms = pg->nperms;
	for ( v = 0; v < (uint16_t)(pg->nperms - 1); perm_sets[v] = (uint16_t)(v + 1), v++ );
	for ( ; v < g->num_vert; perm_sets[v++] = UINT16_MAX );
	leaders[0] = TRUE;
	for ( v = 1; v < g->num_vert; leaders[v++] = FALSE );
	tor = VERTEX;
#ifdef RF
	sp->partition[level].mm_collection = malloc ( sizeof ( MismatchCollection ) );
/*	sp->partition[level].mm_collection->mismatch_hash = malloc ( pc_size * sizeof ( uint16_t ) ); */
	sp->partition[level].mm_collection->mismatch_hash = calloc ( pc_size, sizeof ( uint16_t ) );
	sp->partition[level].mm_collection->counters = calloc ( pc_size, sizeof ( uint16_t ) );
	sp->partition[level].mm_collection->nom = pc_size;
#endif
	for ( v = (uint16_t) ( pivot + 1 ); v < ( pivot + pc_size ); v++ )
	{
		CHECK_IF_VALID;
		/*CHECK_VALID ( sp->partition[level].vertices, &(pg->orbits) );*/
		if ( valid )
		{
			sp->nodes++;
			if ( generate_automorphism ( level, sp->partition[level].vertices[v], sp->partition[level].vertices, g, sp, pg, perm_sets, leaders ) > level )
			{
				add_perm ( pg, sp->discarded_vert, sp->partition[level].nopdv );
				perm_sets[pg->nperms-2] = (uint16_t)(pg->nperms - 1);
			}
			else
			{
				sp->bad_nodes++;
				tor = BACKTR;
#ifdef RF
				mark_new_mismatch ( &(sp->partition[level]), mismatch_marked, go->vertices, v );
#endif
			}
			if( sp->nodes >= sp->nodes_limit )
			{
				printf( "Nodes limit:\t%"PRIu32"\nNodes:\t\t%"PRIu32"\n", sp->nodes_limit, sp->nodes );
				exit(137); /* TIMEOUT */
			}
			for ( i = previous_nperms; i < (pg->nperms - 1); perm_sets[i] = (uint16_t)(i+1), i++ );
			previous_nperms = pg->nperms;
		}
#ifdef RF
		else
		{
			mark_old_mismatch ( &(sp->partition[level]), mismatch_marked, go->vertices, v );
		}
#endif
	}
	if ( tor == BACKTR )
	{
#ifdef RF
		compute_mismatches ( sp->partition[level].mm_collection );
#endif
		sp->partition[level].go = malloc ( sizeof ( Graph_Orbits ) );
		sp->partition[level].go->vertices = malloc ( g->num_vert * sizeof ( Info_Vertex ) );
		sp->partition[level].go->orbits = malloc ( g->num_vert * sizeof ( Info_Orbit ) );
		memcpy ( sp->partition[level].go->vertices, pg->orbits.vertices, g->num_vert * sizeof ( Info_Vertex ) );
		memcpy ( sp->partition[level].go->orbits, pg->orbits.orbits, g->num_vert * sizeof ( Info_Orbit ) );
		sp->partition[level].go->size = pg->orbits.size;
	}
#ifdef RF
	else
	{
		free ( sp->partition[level].mm_collection->mismatch_hash );
		free ( sp->partition[level].mm_collection->counters );
		free ( sp->partition[level].mm_collection );
		sp->partition[level].mm_collection = NULL;
	}
#endif
	memset ( count_orbits, 0, g->num_vert * sizeof ( uint8_t ) );
	for ( v = pivot; v < ( pivot + pc_size ); count_orbits[orbit_of(sp->partition[level].vertices[v++])] = 1 );
	for ( sp->partition[level].no = 0, v = 0; v < g->num_vert; sp->partition[level].no = (uint16_t)( sp->partition[level].no + count_orbits[v++] ) );
	return tor;
}

void find_automorphisms ( const struct graph *g, SeqPart *sp, Perm_Group *pg )
{
	int level;

	set_base ( pg, sp->discarded_vert );
	for ( level = sp->lp; level >= 0; level-- )
	{
		process_cells_with_no_links ( &(sp->partition[level]), pg );
		if ( sp->partition[level].tor == UNKNOWN )
			sp->partition[level].tor = check_automorphism ( g, sp, pg, level );
	}
}

int match ( int level, uint16_t *old_vertices, const struct graph *h, Perm_Group *pg, const struct graph *g, SeqPart *sp, Graph_Orbits *go, uint16_t *iso, uint16_t *perm_sets, uint16_t *leaders )
{
	const uint16_t pc = sp->partition[level].pivot;	/* pivot cell */
	const uint16_t pc_size = (uint16_t)(sp->partition[level].start[pc+1] - sp->partition[level].start[pc]); /* pivot cell size */
	uint16_t new_vertices[sp->partition[level].start[sp->partition[level].noc]];
	uint16_t v;
	int result;

	switch ( sp->partition[level].tor )
	{
		case LAST:
			if ( last_part_compat ( &(sp->partition[level]), old_vertices, g, h ) )
				return ( level );
			break;
		case VERTEX:
			v = sp->partition[level].start[pc];
			if ( vertex_ref_compat ( sp, h, level, old_vertices[v], new_vertices, old_vertices, iso ) )
				if ( ( result = match ( (level + 1), new_vertices, h, pg, g, sp, go, iso, perm_sets, leaders ) ) != level )
					return ( result );
#ifdef RF
			if ( pc_size > 1 )
				mismatch_found_hash = UINT16_MAX;
#endif
			break;
		case SET:
			if ( set_ref_compat ( sp, h, level, new_vertices, old_vertices, iso ) )
				if ( ( result = match ( (level + 1), new_vertices, h, pg, g, sp, go, iso, perm_sets, leaders ) ) != level )
					return ( result );
			break;
		case BACKTR:
		{
			uint8_t valid;
#ifdef RF
			uint16_t counters[pc_size];	/* counters of mismatches */
#endif
			uint16_t new_perm_sets[pg->nelem];
			uint16_t new_leaders[pg->nelem];
			uint16_t w;
#ifdef AGC
			uint16_t valid_vertices[pc_size];
			uint16_t nof_valid;	/* number of valid vertices */
			Info_Vertex info_vertex[pc_size];
			Info_Orbit info_orbit[pc_size];
			Graph_Orbits pgo = { info_vertex, info_orbit, pc_size }; /* partial Graph_Orbits */

/*			if ( sp->partition[level].no != pc_size ) */
			generate_partial_orbits ( sp, level, old_vertices, pg, &pgo, 0, iso, perm_sets, leaders, new_perm_sets, new_leaders );
#endif
#ifdef RF
			for ( v = 0; v < pc_size; counters[v++] = 0 );
#endif
/*			for ( v = sp->partition[level].start[pc]; v < sp->partition[level].start[pc+1]; v++ ) */
#ifdef AGC
			nof_valid = compute_partial_orbits_order ( level, sp, sp->partition[level].go, &pgo, /*old_vertices,*/ valid_vertices );
			for ( w = 0; w < nof_valid; w++ )
#else
			for ( w = 0; w < pc_size; w++ )
#endif
			{
#ifdef AGC
				v = (uint16_t)(sp->partition[level].start[pc] + valid_vertices[w]);
#else
				uint16_t i;

				v = (uint16_t)(sp->partition[level].start[pc] + w);
#endif
#ifdef AGC
				valid = ( pg->orbits.orbits[pg->orbits.vertices[old_vertices[v]].belongs_to].size == orbit_size(sp->partition[level].vertices[sp->partition[level].start[pc]]) );
				if ( valid && ( sp->partition[level].no != pc_size ) )
					CHECK_VALID( old_vertices, &pgo );
#else
				for ( i = 0; i < w && !are_in_the_same_orbit ( old_vertices[sp->partition[level].start[pc]+i], old_vertices[v], go ); i++ );
				valid = ( i == w );
#endif
				if ( valid )
				{
					if ( vertex_ref_compat ( sp, h, level, old_vertices[v], new_vertices, old_vertices, iso ) )
						if ( ( result = match ( ( level + 1 ), new_vertices, h, pg, g, sp, go, iso, new_perm_sets, new_leaders ) ) != level )
							return ( result );
#ifdef RF
					if ( !mismatch_in_collection ( sp->partition[level].mm_collection, counters, 1 ) )
						break;
#endif
				}
			}
#ifdef RF
			mismatch_found_hash = UINT16_MAX;
#endif
			break;
		}
	}
	return ( sp->partition[level].backtrack_level );
}
