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

#ifndef _GROUPS_H_
#define _GROUPS_H_

#include <stdint.h>

#include "orbits.h"

typedef uint16_t* Permutation;

typedef struct
{
	uint16_t nperms;
	uint16_t nelem;
	Permutation base;
	Permutation inv_base;
	Permutation* perms;
	Permutation* inv_perms;
	Graph_Orbits orbits;
} Perm_Group;

Permutation get_perm ( uint16_t n, Perm_Group *pg );

Graph_Orbits* get_graph_orbits ( Perm_Group *pg );

int orbit ( uint16_t vertex, Perm_Group *pg );

int equivalent ( uint16_t v1, uint16_t v2, Perm_Group *pg );

void free_perm_group ( Perm_Group *pg );

void create_perm_group ( uint16_t nelem, Perm_Group *pg );

Permutation get_base ( Perm_Group *pg );

void set_base ( Perm_Group *pg, Permutation p );

void add_perm ( Perm_Group *pg, Permutation p, uint16_t offset );

void set_perm ( Perm_Group *pg, Permutation p );

void inv_perm_partialy ( uint16_t *perm, uint16_t *inv, uint16_t size, uint16_t start, uint16_t end, uint16_t pivot );

uint8_t fixed_in_2_perms ( Perm_Group *pg, uint16_t p1, uint16_t p2, Permutation discarded_vert, uint16_t start, uint16_t end );

int perm_in_pg ( Perm_Group *pg, Permutation p );

#endif
