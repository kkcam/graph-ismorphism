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

/* data structures and funtion definitions to manage integers as products of
   prime powers
*/

#include "factor.h"
/* due to use EXCHANGE macro */
#include "mismatches.h"
#include "seqpart.h"

uint16_t gcd ( uint16_t a, uint16_t b )
{
	uint16_t t;

	while ( b != 0 )
	{
		t = b;
		b = a % b;
		a = t;
	}
	return a;
}

void factMultiply ( const FactoredInt *a, const FactoredInt *b, FactoredInt *r )
{
	uint16_t i, j, k;

	i = j = k = 0;
	while ( i < a->nof || j < b->nof )
	{	if ( j >= b->nof || ( i < a->nof && a->prime[i] < b->prime[j] ) )
		{	r->prime[k] = a->prime[i];
			r->exp[k++] = a->exp[i++];
		}
		else if ( i >= a->nof || ( j < b->nof && a->prime[i] > b->prime[j] ) )
		{	r->prime[k] = b->prime[j];
			r->exp[k++] = b->exp[j++];
		}
		else
		{	r->prime[k] = a->prime[i];
			r->exp[k++] = (uint16_t)(a->exp[i++] + b->exp[j++]);
		}
	}
	r->nof = k;
}

void factDivide ( const FactoredInt *a, const FactoredInt *b, FactoredInt *r )
{
	uint16_t i, j, k;

	i = j = k = 0;
	while ( i < a->nof || j < b->nof )
	{	if ( j >= b->nof || ( i < a->nof && a->prime < b->prime ) )
		{	r->prime[k] = a->prime[i];
			r->exp[k++] = a->exp[i++];
		}
		else if ( i >= a->nof || ( j < b->nof && a->prime[i] > b->prime[j] ) )
		{	r->prime[k] = b->prime[j];
			r->exp[k++] = -(b->exp[j++]);
		}
		else
		{
			r->prime[k] = a->prime[i];
			r->exp[k++] = (uint16_t)(a->exp[i++] - b->exp[j++]);
		}
	}
	r->nof = k;
}

void factorize_factorial ( uint16_t n, FactoredInt *r )
{
	FactoredInt f1, f2;
	FactoredInt *f1_p, *f2_p, *r_p;
	uint16_t i;

	f1_p = &f1;
	f2_p = &f2;
	r_p = r;
	factorize ( 1, r_p );
	for ( i = 2; i <= n; i++ )
	{
		EXCHANGE( f2_p, r_p, FactoredInt* );
		factorize ( i, f1_p );
		factMultiply ( f1_p, f2_p, r_p );
	}
	if ( r != r_p )
		*r = f2;
}

void factorize ( uint16_t n, FactoredInt *r )
{
	static const uint16_t primeList[] =
		{ 2,3,5,7,9,11,13,17,19,23,29,31,37,41,43,47,53,59,
		  61,67,71,73,79,83,89,97,101,103,107,109,113,127, 131,
		  137,139,149,151,157,163,167,173,179,181,191,193,197,
		  199,211,223,227,229,233,239,241,251,0 };
	uint16_t i;

	i = 0;
	r->nof = 0;
	while ( primeList[i] * primeList[i] <= n )
	{	r->prime[r->nof] = primeList[i];
		r->exp[r->nof] = 0;
		while ( n % primeList[i] == 0 )
		{	r->exp[r->nof]++;
			n = n / primeList[i];
		}
		i++;
		if ( r->exp[r->nof] > 0 )
			r->nof++;
	} 
	if ( n > 1 )
	{
		r->prime[r->nof] = n;
		r->exp[r->nof++] = 1;
	}
}

int factEqual ( const FactoredInt *a, const FactoredInt *b )
{	uint16_t i;

	if ( a->nof != b->nof )
		return 0; /* FALSE */
	for ( i = 0; i < a->nof && a->prime[i] == b->prime[i] && a->exp[i] == b->exp[i]; i++ );
	return ( i == a->nof );
}
