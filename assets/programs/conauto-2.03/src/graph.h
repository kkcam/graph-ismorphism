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

/* Graph representation and graph file operations */

#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define NOT_ADJ 0
#define ARC_IN 1
#define ARC_OUT 2
#define ARC_IO 3

struct graph {
	uint64_t *deg;
	int8_t **adj;
	uint16_t num_vert;
	uint32_t num_arc;
};

void read_graph_sivalab ( struct graph *g, FILE *f );

void read_graph_dimacs ( struct graph *g, FILE *f );

#endif

