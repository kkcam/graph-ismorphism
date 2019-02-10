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

#include <stdlib.h>
#include <inttypes.h>

#include "graph.h"

uint16_t read_word ( FILE *f )
{
	int b1, b2;

	b1 = fgetc ( f );
	b2 = fgetc ( f );
	return (uint16_t) ( b1 | ( b2 << 8 ) );
}

void read_graph_sivalab ( struct graph *g, FILE *f )
{
	uint16_t from, to;
	uint16_t num_arc;
	uint16_t i;

	g->num_vert = read_word ( f );
	g->deg = calloc ( (size_t)(g->num_vert), sizeof(uint64_t) );
	g->adj = calloc ( (size_t)(g->num_vert), sizeof(uint8_t*) );
	for ( from = 0; from < g->num_vert; from++ )
	{
		g->adj[from] = calloc ( (size_t)(g->num_vert), sizeof(uint8_t) );
		for ( to = 0; to < g->num_vert; to++ )
			g->adj[from][to] = NOT_ADJ;
		g->deg[from] = (uint64_t)0;
	}
	g->num_arc = 0;
	for ( from = 0; from < g->num_vert; from++ )
	{ 
		num_arc = read_word ( f );
		for ( i = 0; i < num_arc; i++ )
		{
			to = read_word ( f );
			g->num_arc++;
			if ( g->adj[from][to] == NOT_ADJ )
			{
				g->deg[from] = g->deg[from] + UINT64_C(0x100000000) + (uint64_t)(from!=to);
				g->deg[to] = g->deg[to] + UINT64_C(0x10000) + (uint64_t)(from!=to);
			}
			else
			{
				g->deg[from] = g->deg[from] - UINT64_C(0x10000) + UINT64_C(0x1000000000000);
				g->deg[to] = g->deg[to] - UINT64_C(0x100000000) + UINT64_C(0x1000000000000);
			}
			g->adj[from][to] = g->adj[from][to] | ARC_OUT;
			g->adj[to][from] = g->adj[to][from] | ARC_IN;
		}
	}
}

void read_graph_dimacs ( struct graph *g, FILE *f )
{
	const int buff_size = 1024;
	char line[buff_size];
	uint16_t from, to;
	uint32_t i;

	/* delete comments */
	do {
		fgets( line, buff_size, f );
	} while( strncmp( line, "c ", 2 ) == 0 );
	sscanf( line, "p edge %"SCNu16" %"SCNu32"\n", &(g->num_vert), &(g->num_arc) );
	g->deg = calloc ( (size_t)(g->num_vert), sizeof(uint64_t) );
	g->adj = calloc ( (size_t)(g->num_vert), sizeof(uint8_t*) );
	for ( from = 0; from < g->num_vert; from++ )
	{
		g->adj[from] = calloc ( (size_t)(g->num_vert), sizeof(uint8_t) );
		for ( to = 0; to < g->num_vert; to++ )
			g->adj[from][to] = NOT_ADJ;
		g->deg[from] = (uint64_t)0;
	}
	for( i = 0; i < g->num_arc; i++ )
	{
		/* delete comments */
		do {
			fgets( line, buff_size, f );
		} while( strncmp( line, "c ", 2 ) == 0 );
		sscanf( line, "e %"SCNu16" %"SCNu16"\n", &from, &to );
		from = from - 1;
		to = to - 1;
		if ( g->adj[from][to] == NOT_ADJ )
		{
			g->deg[from] = g->deg[from] + UINT64_C(0x100000000) + (uint64_t)(from!=to);
			g->deg[to] = g->deg[to] + UINT64_C(0x10000) + (uint64_t)(from!=to);
		}
		else
		{
			g->deg[from] = g->deg[from] - UINT64_C(0x10000) + UINT64_C(0x1000000000000);
			g->deg[to] = g->deg[to] - UINT64_C(0x100000000) + UINT64_C(0x1000000000000);
		}
		g->adj[from][to] = g->adj[from][to] | ARC_OUT;
		g->adj[to][from] = g->adj[to][from] | ARC_IN;
	}
}

