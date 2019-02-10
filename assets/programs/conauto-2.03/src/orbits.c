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

/* Data structures and functions for orbit management */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#include "orbits.h"
/*
#include "seqpart.h"
*/

#define TRUE 1
#define FALSE 0

void free_graph_orbits ( Graph_Orbits *go )
{
	free ( go->vertices );
	free ( go->orbits );
}

void allocate_graph_orbits ( uint16_t size, Graph_Orbits *go )
{
	go->size = size;
	go->vertices = malloc ( size * sizeof(Info_Vertex) );
	go->orbits = malloc ( size * sizeof(Info_Orbit) );
}


void create_trivial_orbits ( Graph_Orbits *go )
{
	uint16_t i;

	for ( i = 0; i < go->size; i++ )
	{
		go->vertices[i].belongs_to = i;
		go->vertices[i].next = UINT16_MAX;
		go->orbits[i].first = i;
		go->orbits[i].last = i;
		go->orbits[i].size = 1;
	}
}

#define exchange(a,b) { a = a ^ b; b = a ^ b; a = a ^ b; }

void merge_orbits ( Graph_Orbits *go, uint16_t v1, uint16_t v2 )
{
	uint16_t v;
	if ( same_orbit(v1,v2) )
		return;
	if ( orbit_size(v1) > orbit_size(v2) )
		exchange(v1,v2)
	orbit_size(v2) = (uint16_t) ( orbit_size(v2) + orbit_size(v1) );
	orbit_size(v1) = 0;
	go->vertices[go->orbits[orbit_of(v2)].last].next = go->orbits[orbit_of(v1)].first;
	go->orbits[orbit_of(v2)].last = go->orbits[orbit_of(v1)].last;
	for ( v = go->orbits[orbit_of(v1)].first; v != UINT16_MAX; v = go->vertices[v].next )
		orbit_of(v) = orbit_of(v2);
}

void destroy_graph_orbits ( Graph_Orbits *go )
{
	free ( go->orbits );
	free ( go->vertices );
}

uint16_t are_in_the_same_orbit ( uint16_t v1, uint16_t v2, Graph_Orbits *go )
{
	return same_orbit(v1,v2);
}

void mark_not_valid ( uint8_t *valid, Graph_Orbits *go, uint16_t vertex )
{
	uint16_t v;

	for ( v = go->orbits[orbit_of(vertex)].first; v != UINT16_MAX; v = go->vertices[v].next )
		valid[v] = FALSE;
}

uint16_t get_nof_orbits ( Graph_Orbits *go )
{
	uint8_t shown[go->size];
	uint16_t v,w;
	uint16_t orb_count;

	orb_count = 0;
	for ( v = 0; v < go->size; v++ )
		shown[v] = FALSE;
	for ( v = 0; v < go->size; v++ )
		if ( ! shown[v] )
		{
			orb_count++;
			for ( w = go->orbits[orbit_of(v)].first; w != UINT16_MAX; w = go->vertices[w].next )
				shown[w] = TRUE;
		}
	return orb_count;
}

