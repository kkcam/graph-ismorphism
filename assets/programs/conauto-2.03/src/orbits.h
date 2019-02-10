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

#ifndef _ORBITS_H_
#define _ORBITS_H_

#include <stdint.h>

typedef struct {
	uint16_t belongs_to;
	uint16_t next;
} Info_Vertex;

typedef struct {
	uint16_t first;
	uint16_t last;
	uint16_t size;
} Info_Orbit;

typedef struct {
	Info_Vertex *vertices;
	Info_Orbit *orbits;
	uint16_t size;
} Graph_Orbits;

#define orbit_of(v) go->vertices[v].belongs_to
#define same_orbit(v,w) ( orbit_of(v) == orbit_of(w) )
#define orbit_size(v) go->orbits[orbit_of(v)].size

void free_graph_orbits ( Graph_Orbits *go );

void allocate_graph_orbits ( uint16_t size, Graph_Orbits *go );

void create_trivial_orbits ( Graph_Orbits *go );

void merge_orbits ( Graph_Orbits *go, uint16_t v1, uint16_t v2 );

void destroy_graph_orbits ( Graph_Orbits *go );

uint16_t are_in_the_same_orbit ( uint16_t v1, uint16_t v2, Graph_Orbits *go );

void mark_not_valid ( uint8_t *valid, Graph_Orbits *go, uint16_t vertex );

uint16_t get_nof_orbits ( Graph_Orbits *go );

#endif

