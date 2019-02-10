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

/* conauto */

# include <sys/time.h>
# include <sys/resource.h>
# include <stdint.h>
# include <unistd.h>
# include <stdio.h>
# include <inttypes.h>
# include <string.h>

# include "graph.h"
# include "groups.h"
# include "mismatches.h"
# include "seqpart.h"
# include "factor.h"

#define ERROR 1
#define FALSE 0
#define TRUE 1

#define SIVALAB 0
#define DIMACS 1
/*******************************************************
* Avilable readable types of graphs:
* 	0: sivalab
*******************************************************/
uint8_t g_type;	/* graph type */
/* times flag */
uint8_t iso_t_flag;
uint8_t sp_t_flag;
uint8_t aut_t_flag;
uint8_t mtx_t_flag;
uint8_t info_flag;
uint8_t auto_flag;
double auto_time;
uint16_t multiplicity=1;
uint32_t nodes_limit=UINT32_MAX;

struct timeval t,u,v,w,x;

void default_params ( void )
{
	iso_t_flag = FALSE;
	sp_t_flag = FALSE;
	aut_t_flag = FALSE;
	mtx_t_flag = FALSE;
	g_type = SIVALAB;
	info_flag = FALSE;
	auto_flag = FALSE;
	auto_time = 0.0;
}

void show_help ( int exit_type )
{
/*	printf ( "usage: \tconauto [-t] graph1 [-g type] graph2 [-h type]\n" ); */
	printf ( "usage: \tconauto-2.0 [-t TIME_TYPE[,TIME_TYPE,...]] [-dv] graph1 graph2\n"
		"\tconauto-2.0 [-t all] [-dv] graph1 graph2\n"
		"\tconauto-2.0 -aut [-M=X] [-N=Y] [-d] graph\n" );
	printf ( "help: \tconauto-2.0 --\n" );
	printf ( "-v: verbose\n" );
	printf ( "-aut: compute automorphism group\n" );
	printf ( "-d: read the input graph(s) as DIMACS format\n" );
	printf ( "-M=X: multiplicity, where X is the multiplicity value (default: X=1)\n" );
	printf ( "-N=Y: search tree nodes limit, where Y is the limit value (default: infinite)\n" );
	printf ( "-t: Show times\n" );
	printf ( "TIME_TYPEs: \n\t"
		"iso: show time elapsed in isomorphism test\n\t"
		" sp: show time elapsed in generation of sequence of partitions\n\t"
		"aut: show time elapsed in search of automorphisms\n\t"
		"mtx: show time elapsed in match both graphs\n\t"
		"all: show all times\n" );
	_exit ( exit_type );
}

void proc_times ( char *optarg )
{
	char tokenizer[2] = ",";
	char *token;
	
	token = strtok ( optarg, tokenizer );
	do
	{
		if ( strcmp ( token, "iso") == 0 )
			iso_t_flag = TRUE;
		else if ( strcmp ( token, "sp") == 0 )
			sp_t_flag = TRUE;
		else if ( strcmp ( token, "aut") == 0 )
			aut_t_flag = TRUE;
		else if ( strcmp ( token, "mtx") == 0 )
			mtx_t_flag = TRUE;
		else
			show_help ( ERROR );
	} while ( ( token = strtok ( NULL, tokenizer ) ) != NULL );
}

void proc_params ( int argc, char *argv[] )
{
	int param;

	if ( argc < 2 )
		show_help ( ERROR );
	default_params ();
	/* automorphism params */
	if ( ( strcmp ( argv[1], "-aut" ) == 0 ) )
	{
		auto_flag = TRUE;
		for ( param = 2; param < argc; param++)
			if ( strncmp( argv[param], "-M=", 3 * sizeof( char ) ) == 0 )
				sscanf ( argv[param]+3, "%"SCNu16, &multiplicity );
			else if ( strncmp( argv[param], "-N=", 3 * sizeof( char ) ) == 0 )
				sscanf ( argv[param]+3, "%"SCNu32, &nodes_limit );
			else if ( strncmp( argv[param], "-d", 2 * sizeof( char ) ) == 0 )
				g_type = DIMACS;
		return;
	}
	/* isomorphism params */
	while ( ( param = getopt ( argc, argv, "d:t:-v" ) ) != -1 )
	{
		switch ( param )
		{
			case 't': /* show times */
				if ( strcmp ( optarg, "all" ) == 0 )
				{
					iso_t_flag = TRUE;
					sp_t_flag = TRUE;
					aut_t_flag = TRUE;
					mtx_t_flag = TRUE;
				}
				else
					proc_times( optarg );
				break;
			case 'v':	/* verbose */
				info_flag = TRUE;
				break;
			case 'd': /* graph type: DIMACS */
				g_type = DIMACS;
				break;
			case '-': /* help */
				show_help( 0 );
				break;
			default:
				break;
		}
	}
	if ( argc < 3 )
		show_help ( ERROR );
}

double difference ( struct timeval *ti, struct timeval *tf )
{
	double d;

	if ( tf->tv_usec < ti->tv_usec )
	{
		tf->tv_sec--;
		tf->tv_usec += 1000000;
	}
	d = (double)( tf->tv_sec - ti->tv_sec ) + ( (double) ( tf->tv_usec - ti->tv_usec ) ) / 1000000;
	return ( d );
}

FactoredInt compute_pg_size ( SeqPart *sp )
{
	FactoredInt f1, f2, f3;
	FactoredInt *f_result, *f_size, *f_aux;
	uint16_t size;
	uint16_t level;

	f_result = &f1;
	f_size = &f2;
	f_aux = &f3;
	factorize ( 1, f_result );
	for ( level = 0; level <= sp->lp; level++ )
	{
		/* factorize cells without links */
		if ( sp->partition[level].nodc > 0 )
		{
			uint16_t c;

			for ( c = 0; c < sp->partition[level].nodc; c++ )
				if ( ( size = (uint16_t)(sp->partition[level].start[sp->partition[level].discarded_cells[c]+1] - sp->partition[level].start[sp->partition[level].discarded_cells[c]]) ) > 1 )
				{
					factorize_factorial ( size, f_size );
					factMultiply ( f_size, f_result, f_aux );
					EXCHANGE ( f_result, f_aux, FactoredInt* );
				}
		}
		/* factorize individualized cells */
		if ( sp->partition[level].tor == BACKTR )
			size = sp->partition[level].go->orbits[sp->partition[level].go->vertices[sp->partition[level].vertices[sp->partition[level].start[sp->partition[level].pivot]]].belongs_to].size;
		else if ( (sp->partition[level].tor == VERTEX ) && ( ( size = (uint16_t)(sp->partition[level].start[sp->partition[level].pivot+1] - sp->partition[level].start[sp->partition[level].pivot]) ) > 1 ) ); /* implicit assignation in 'size' variable */
		/* SET or VERTEX levels */
		else
			continue;
		factorize ( size, f_size );
		factMultiply ( f_size, f_result, f_aux );
		EXCHANGE ( f_result, f_aux, FactoredInt* );
	}
	if ( f_result->nof == 0 )
	{
		f_result->prime[0] = 1;
		f_result->exp[0] = 1;
	}
	return *f_result;
}

void show_info ( const struct graph *g, SeqPart *sp, Perm_Group *pg )
{
	FactoredInt f_result;
	uint16_t factors;

	f_result = compute_pg_size ( sp );
	printf ( "Vertices: \t%"PRIu16"\n", g->num_vert );
	printf ( "Edges: \t\t%"PRIu16"\n", g->num_arc );
	printf ( "|Aut|:\t\t%"PRIu16, f_result.prime[0] );
	if( f_result.exp[0] > 1 )
		printf( "^%"PRIu16, f_result.exp[0] );
	for ( factors = 1; factors < f_result.nof; factors++ )
	{
		printf ( " * %"PRIu16, f_result.prime[factors] );
		if( f_result.exp[factors] > 1 )
			printf( "^%"PRIu16, f_result.exp[factors] );
	}
	printf ( "\n" );
	printf ( "Nodes: \t\t%"PRIu32"\n", sp->nodes + sp->dcs_nodes );
	printf ( "Bad nodes: \t%"PRIu32"\n", sp->bad_nodes );
	printf ( "Orbits: \t%"PRIu16"\n", get_nof_orbits ( &(pg->orbits) ) );
	printf ( "Generators: \t%"PRIu16"\n", pg->nperms-1 );
	printf ( "--------------------------\n" );
}

void compute_automrphism_group ( const struct graph *g, SeqPart *sp, Perm_Group *pg, const char s[] )
{
	if ( sp_t_flag )
		gettimeofday ( &v, NULL );
	generate_seqpart ( sp, g, 0 );
	if ( sp_t_flag )
	{
		gettimeofday ( &w, NULL );
		printf ( "Generation of the sequence of partitions of %s (seconds): %f\n", s, difference ( &v, &w ) );
	}
	compute_backtrack_levels ( sp );
#ifdef EAD
	compute_auto_search_limits ( sp );
#endif
	if ( aut_t_flag )
		gettimeofday ( &v, NULL );
	find_automorphisms ( g, sp, pg );
	if ( aut_t_flag )
	{
		gettimeofday ( &w, NULL );
		printf ( "Search of automorphisms graph %s (seconds): %f\n", s, difference ( &v, &w ) );
	}
	if ( info_flag )
		show_info ( g, sp, pg );
}

void compute_automorphisms ( const struct graph *g )
{
	Perm_Group pg;	/* Permutation Group */
	SeqPart sp;	/* Sequence of Partitions */
	uint16_t m;

	gettimeofday ( &v, NULL );
	for ( m = 0; m < multiplicity; m++ )
	{
		allocate_seqpart ( &sp, g->num_vert );
		sp.nodes_limit = nodes_limit;
		generate_degree_partition ( &sp, g );
		create_perm_group ( g->num_vert, &pg );
		compute_automrphism_group ( g, &sp, &pg, "g" );
		if ( m+1 != multiplicity )
		{
			free_seqpart ( &sp );
			free_perm_group ( &pg );
		}
	}
	gettimeofday ( &w, NULL );
	auto_time = difference ( &v, &w );
	/* implicit auto_flag=TRUE */
	show_info ( g, &sp, &pg );
	printf ( "seconds: %f\n", auto_time/multiplicity );
}

int are_isomorphic ( const struct graph *g, const struct graph *h )
{
	Perm_Group pg_g, pg_h;	/* Permutation Groups */
	SeqPart sp_g, sp_h;	/* Sequence of Partitions */
	FactoredInt fi_g, fi_h;
	uint16_t nobp_g, nobp_h; /* number of bracktracking points */
	uint16_t iso[g->num_vert];
	uint16_t perm_sets[g->num_vert];
	uint16_t leaders[g->num_vert];
	uint16_t p;

	if ( ( g->num_vert != h->num_vert ) || ( g->num_arc != h->num_arc ) )
	{
		mtx_t_flag = FALSE;
		return FALSE;
	}
	allocate_seqpart ( &sp_g, g->num_vert );
	allocate_seqpart ( &sp_h, h->num_vert );
	generate_degree_partition ( &sp_g, g );
	generate_degree_partition ( &sp_h, h );
	if ( ! degree_partitions_are_compatible ( &(sp_g.partition[0]), &(sp_h.partition[0]) ) )
	{
		mtx_t_flag = FALSE;
		return FALSE;
	}
	create_perm_group ( g->num_vert, &pg_g );
	create_perm_group ( h->num_vert, &pg_h );
	compute_automrphism_group ( g, &sp_g, &pg_g, "g" );
	if ( 1 > ( nobp_g = num_backtrack_levels ( &sp_g ) ) )
	{
		if ( mtx_t_flag )
			gettimeofday ( &x, NULL );
		return ( 0 <= match ( 0, sp_h.partition[0].vertices, h, &pg_h, g, &sp_g, &(pg_g.orbits), iso, NULL, NULL ) );
	}
	compute_automrphism_group ( h, &sp_h, &pg_h, "h" );
	nobp_h = num_backtrack_levels ( &sp_h );
#ifdef AGC
	fi_g = compute_pg_size ( &sp_g );
	fi_h = compute_pg_size ( &sp_h );
	if ( !factEqual ( &fi_g, &fi_h ) || get_nof_orbits ( &(pg_g.orbits) ) != get_nof_orbits ( &(pg_h.orbits) ) )
	{
		mtx_t_flag = FALSE;
		return FALSE;
	}
#endif
	if ( mtx_t_flag )
		gettimeofday ( &x, NULL );
	if ( nobp_g <= nobp_h )
	{
		for ( p = 0; p < (uint16_t)(pg_h.nperms - 1); perm_sets[p] = (uint16_t)(p + 1), p++ );
		perm_sets[pg_h.nperms - 1] = UINT16_MAX;
		leaders[0] = TRUE;
		for ( p = 1; p < pg_h.nperms; leaders[p++] = FALSE );

		return ( 0 <= match ( 0, sp_h.partition[0].vertices, h, &pg_h, g, &sp_g, &(pg_g.orbits), iso, perm_sets, leaders ) );
	}
	else
	{
		for ( p = 0; p < (uint16_t)(pg_g.nperms - 1); perm_sets[p] = (uint16_t)(p + 1), p++ );
		perm_sets[pg_g.nperms - 1] = UINT16_MAX;
		leaders[0] = TRUE;
		return ( 0 <= match ( 0, sp_g.partition[0].vertices, g, &pg_g, h, &sp_h, &(pg_h.orbits), iso, perm_sets, leaders ) );
	}
}

int main ( int argc, char *argv[] )
{
	FILE *fg, *fh;
	struct graph g, h;
	struct timeval t, u;
	struct rlimit rl;
	int iso;

	proc_params ( argc, argv );
	fg = fh = NULL;
	if ( ! ( fg = fopen ( argv[argc-2 + auto_flag], "rb" ) ) )
	{
		printf ( "Graph %s cannot be opened\n", argv[argc-2] );
		exit ( ERROR );
	}
	if ( !auto_flag && ! ( fh = fopen ( argv[argc-1], "rb" ) ) )
	{
		printf ( "Graph %s cannot be opened\n", argv[argc-1] );
		exit ( ERROR );
	}
	if( g_type == SIVALAB )
		read_graph_sivalab ( &g, fg );
	else if( g_type == DIMACS )
		read_graph_dimacs ( &g, fg );
	if ( !auto_flag )
	if( g_type == SIVALAB )
		read_graph_sivalab ( &h, fh );
	else if( g_type == DIMACS )
		read_graph_dimacs ( &h, fh );

	/* set stack size */
	getrlimit ( RLIMIT_STACK, &rl );
	rl.rlim_cur = (0x8000000>rl.rlim_max)?rl.rlim_max:0x8000000;
	setrlimit ( RLIMIT_STACK, &rl );

	if ( auto_flag ) /* automorphism group */
		compute_automorphisms ( &g );
	else /* isomorphism */
	{
		if ( iso_t_flag )
			gettimeofday ( &t, NULL );
		iso = are_isomorphic ( &g, &h );
		if ( iso_t_flag || mtx_t_flag )
		{
			gettimeofday ( &u, NULL );
			if ( mtx_t_flag )
				printf ( "Match (seconds): %f\n", difference ( &x, &u ) );
			if ( iso_t_flag )
				printf ( "seconds: %f\n", difference ( &t, &u ) );
		}
		printf ( "The input graphs are%s isomorphic\n", (iso)?"":" NOT" );
	}
	return 0;
}

