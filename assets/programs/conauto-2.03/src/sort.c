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

/* sort function which uses qsort */

#include <stdlib.h>
#include <string.h>

#include "sort.h"

#define EXCHANGE( A, B )\
{	register void* C; C = A; A = B; B = C;\
}

#define MERGE( OLD, NEW, I1, S1, I2, S2 )\
{\
	uint16_t k, l, m;\
	k = I1;\
	l = I2;\
	m = I1;\
	while ( ( k <= S1 ) && ( l <= S2 ) )\
		if ( attributes[OLD[k]] >= attributes[OLD[l]] )\
			NEW[m++] = OLD[k++];\
		else\
			NEW[m++] = OLD[l++];\
	while ( k <= S1 )\
		NEW[m++] = OLD[k++];\
	while ( l <= S2 )\
		NEW[m++] = OLD[l++];\
}

#define COPY( OLD, NEW, DOWN_LIM, UP_LIM )\
{\
	uint32_t k;\
	for ( k = DOWN_LIM; k <= UP_LIM; k++ )\
		NEW[k] = OLD[k];\
}

void sort_cell ( uint16_t nmemb, uint16_t *base, uint64_t *attributes )
{
	uint16_t i1, s1;
	uint16_t i2, s2;
	uint16_t size;
	uint16_t temp[nmemb];
	uint16_t *new, *old;
	uint16_t up_to;

	old = base;
	new = temp;
	up_to = (uint16_t) (nmemb-1);
	for ( size = 1; size < nmemb; size = (uint16_t) (size << 1 ) )
	{
		i1 = 0;
		i2 = (uint16_t) ( i1 + size );
		s1 = (uint16_t) ( i2 - 1 );
		s2 = (uint16_t) ( s1 + size );
		while ( s2 < nmemb )
		{
			MERGE ( old, new, i1, s1, i2, s2 );
			i1 = (uint16_t) ( s2 + 1 );
			i2 = (uint16_t) ( i1 + size );
			s1 = (uint16_t) ( i2 - 1 );
			s2 = (uint16_t) ( s1 + size );
		}
		if ( s1 < up_to )
			MERGE ( old, new, i1, s1, i2, up_to )
		else
			COPY ( old, new, i1, up_to )
		EXCHANGE ( old, new );
	}
	if ( old != base )
		memcpy ( base, old, nmemb * sizeof ( uint16_t ) );
}

#define MERGE_UINT16( OLD, NEW, I1, S1, I2, S2 )\
{\
	uint16_t k, l, m;\
	k = I1;\
	l = I2;\
	m = I1;\
	while ( ( k <= S1 ) && ( l <= S2 ) )\
		if ( OLD[k] >= OLD[l] )\
			NEW[m++] = OLD[k++];\
		else\
			NEW[m++] = OLD[l++];\
	while ( k <= S1 )\
		NEW[m++] = OLD[k++];\
	while ( l <= S2 )\
		NEW[m++] = OLD[l++];\
}

void sort_uint16 ( uint16_t nmemb, uint16_t *base )
{
	uint16_t i1, s1;
	uint16_t i2, s2;
	uint16_t size;
	uint16_t temp[nmemb];
	uint16_t *new, *old;
	uint16_t up_to;

	old = base;
	new = temp;
	up_to = (uint16_t) (nmemb-1);
	for ( size = 1; size < nmemb; size = (uint16_t) (size << 1 ) )
	{
		i1 = 0;
		i2 = (uint16_t) ( i1 + size );
		s1 = (uint16_t) ( i2 - 1 );
		s2 = (uint16_t) ( s1 + size );
		while ( s2 < nmemb )
		{
			MERGE_UINT16 ( old, new, i1, s1, i2, s2 );
			i1 = (uint16_t) ( s2 + 1 );
			i2 = (uint16_t) ( i1 + size );
			s1 = (uint16_t) ( i2 - 1 );
			s2 = (uint16_t) ( s1 + size );
		}
		if ( s1 < up_to )
			MERGE_UINT16 ( old, new, i1, s1, i2, up_to )
		else
			COPY ( old, new, i1, up_to )
		EXCHANGE ( old, new );
	}
	if ( old != base )
		memcpy ( base, old, nmemb * sizeof ( uint16_t ) );
}

#define MERGE_VALID_ORBITS( OLD, NEW, I1, S1, I2, S2 )\
{\
	uint16_t k, l, m;\
	k = I1;\
	l = I2;\
	m = I1;\
	while ( ( k <= S1 ) && ( l <= S2 ) )\
		if ( go->orbits[go->vertices[OLD[k]].belongs_to].size >= go->orbits[go->vertices[OLD[l]].belongs_to].size )\
			NEW[m++] = OLD[k++];\
		else\
			NEW[m++] = OLD[l++];\
	while ( k <= S1 )\
		NEW[m++] = OLD[k++];\
	while ( l <= S2 )\
		NEW[m++] = OLD[l++];\
}

void sort_valid_orbits ( uint16_t nmemb, uint16_t *base, Graph_Orbits *go )
{
	uint16_t i1, s1;
	uint16_t i2, s2;
	uint16_t size;
	uint16_t temp[nmemb];
	uint16_t *new, *old;
	uint16_t up_to;

	old = base;
	new = temp;
	up_to = (uint16_t) (nmemb-1);
	for ( size = 1; size < nmemb; size = (uint16_t) (size << 1 ) )
	{
		i1 = 0;
		i2 = (uint16_t) ( i1 + size );
		s1 = (uint16_t) ( i2 - 1 );
		s2 = (uint16_t) ( s1 + size );
		while ( s2 < nmemb )
		{
			MERGE_VALID_ORBITS ( old, new, i1, s1, i2, s2 );
			i1 = (uint16_t) ( s2 + 1 );
			i2 = (uint16_t) ( i1 + size );
			s1 = (uint16_t) ( i2 - 1 );
			s2 = (uint16_t) ( s1 + size );
		}
		if ( s1 < up_to )
			MERGE_VALID_ORBITS ( old, new, i1, s1, i2, up_to )
		else
			COPY ( old, new, i1, up_to )
		EXCHANGE ( old, new );
	}
	if ( old != base )
		memcpy ( base, old, nmemb * sizeof ( uint16_t ) );
}

#define ROL32(x, n) (((x) << (n)) | ((x) >> (32-(n))))
#define ROL16(x, n) (((x) << (n)) | ((x) >> (16-(n))))

uint32_t hash_Meiyan32 ( const void *key, size_t len )
{
    const uint32_t PRIME = 709607;
    uint32_t hash32;
    const uint32_t *p;

    for ( hash32 = 2166136261, p = key; --len; p++ )
    {
        hash32 = ( hash32 ^ ( ROL32(*p,5) ^ *(p+1) ) ) * PRIME;        
    }
    return hash32 ^ ( hash32 >> 16 );
}
 
uint16_t hash_Meiyan16 ( const void *key, size_t len )
{
    const uint16_t PRIME = 9173;
    uint16_t hash16;
    const uint16_t *p;

    /* for ( hash16 = 40387, p = key; --len; p++ ) */
    for ( hash16 = 43387, p = key; --len; p++ )
    {
	if ( hash16 == ( ROL16(*p,5) ^ *(p+1) ) )
		hash16++;
        hash16 = (uint16_t)(( hash16 ^ ( ROL16(*p,5) ^ *(p+1) ) ) * PRIME);
    }
    return hash16 ^ ( hash16 >> 8 );
}
