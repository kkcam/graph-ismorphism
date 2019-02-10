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

/* operations on permutation groups */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "groups.h"
#include "orbits.h"

#define TRUE 1
#define FALSE 0

Permutation get_perm ( uint16_t n, Perm_Group *pg )
{
	return pg->perms[n];
}

int orbit ( uint16_t vertex, Perm_Group *pg )
{
	const Graph_Orbits *go = &(pg->orbits);

	return orbit_of(vertex);
}

int equivalent ( uint16_t v1, uint16_t v2, Perm_Group *pg )
{
	return are_in_the_same_orbit ( v1, v2, &(pg->orbits) );
}

int all_fixed_fixed ( uint16_t fixed[], Perm_Group *pg, uint16_t p )
{
	int allFixed;
	uint16_t v;

	allFixed = TRUE;
	for ( v = 0; allFixed && fixed[v] != UINT16_MAX; v++);
		allFixed = ( pg->perms[p][fixed[v]] == fixed[v] );
	return allFixed;
}

void free_perm_group ( Perm_Group *pg )
{
	uint16_t i;

	for ( i = 0; i < pg->nperms; i++ )
	{
		free ( pg->perms[i] );
		free ( pg->inv_perms[i] );
	}
	/* pg->base and pg->inv_base already free */
	free ( pg->perms );
	free ( pg->inv_perms );
	free_graph_orbits ( &(pg->orbits) );
}

void create_perm_group ( uint16_t nelem, Perm_Group *pg )
{
	pg->nperms = 0;
	pg->nelem = nelem;
	pg->perms = calloc ( (size_t)nelem, sizeof(Permutation) );
	pg->inv_perms = calloc ( (size_t)nelem, sizeof ( Permutation ) );
	allocate_graph_orbits ( nelem, &(pg->orbits) );
	create_trivial_orbits ( &(pg->orbits) );
}

void multiply_perms ( Permutation perm1, Permutation perm2, Permutation result, uint32_t length )
{
	uint16_t i;

	for ( i=0; i < length; i++)
		result[i] = perm1[perm2[i]];
}

Permutation get_base ( Perm_Group *pg )
{
	return pg->base;
}

void set_inv_perm ( Perm_Group *pg, Permutation *inv_perm, Permutation perm )
{
	uint16_t i;

	*inv_perm = malloc ( pg->nelem * sizeof ( uint16_t ) );
	for ( i = 0; i < pg->nelem; i++ )
		(*inv_perm)[perm[i]] = i;
}

void set_base ( Perm_Group *pg, Permutation p )
{
	pg->base = malloc ( pg->nelem * sizeof ( uint16_t ) );
	memcpy ( pg->base, p, pg->nelem * sizeof ( uint16_t ) ); 
	set_inv_perm ( pg, &(pg->inv_base), pg->base );
	pg->perms[0] = pg->base;
	pg->inv_perms[0] = pg->inv_base;
	pg->nperms = 1;
}

void set_perm ( Perm_Group *pg, Permutation p )
{
	pg->perms[pg->nperms] = malloc ( pg->nelem * sizeof ( uint16_t ) );
	set_inv_perm ( pg, &(pg->inv_perms[pg->nperms]), p );
	memcpy ( pg->perms[pg->nperms++], p, pg->nelem * sizeof ( uint16_t ) );
}

void add_perm ( Perm_Group *pg, Permutation p, uint16_t offset )
{
	uint16_t v;

	set_perm ( pg, p );
	for ( v = offset; v < pg->nelem; v++ )
		if ( pg->base[v] != p[v] )
			merge_orbits ( &(pg->orbits), pg->base[v], p[v] );
}

void inv_perm_partialy ( uint16_t *perm, uint16_t *inv, uint16_t size, uint16_t start, uint16_t end, uint16_t pivot )
{
	uint16_t i;

	for ( i = 0; i < size; inv[i++] = UINT16_MAX );
	for ( i = start; i < end; i++ )
		inv[perm[i]] = i;
	inv[pivot] = UINT16_MAX;
}

uint8_t fixed_in_2_perms ( Perm_Group *pg, uint16_t p1, uint16_t p2, Permutation discarded_vert, uint16_t start, uint16_t end )
{
	uint16_t v;

	for ( v = start; v < end; v++ )
		if ( pg->inv_perms[p1][discarded_vert[v]] != pg->inv_perms[p2][discarded_vert[v]] )
			return FALSE;
	return TRUE;
}

int perm_in_pg ( Perm_Group *pg, Permutation p )
{
	int test;
	uint16_t i;

/*	test = ( memcmp ( pg->base, p, pg->nelem * sizeof ( uint16_t ) ) == 0 );	# now base is in perms array */
	test = FALSE;
	for ( i = 0; i < pg->nperms && !test; i++ )
		test = ( memcmp ( pg->perms[i], p, pg->nelem * sizeof ( uint16_t ) ) == 0 );
	return test;
}

