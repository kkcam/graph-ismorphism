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

#include <stdio.h>
#include <string.h>

#include "mismatches.h" 
#include "sort.h"

#define TRUE 1
#define FALSE 0

void mark_new_mismatch ( Partition *p, uint8_t *mm_marked, Info_Vertex *info_vert, uint16_t mmv )
{
	const MismatchCollection *mm = p->mm_collection;
	uint16_t v;

	for ( v = mmv; v < p->start[p->pivot+1]; v++ )
		if ( ( info_vert[p->vertices[mmv]].belongs_to == info_vert[p->vertices[v]].belongs_to ) && !mm_marked[v - p->start[p->pivot]] )
		{
			mm->mismatch_hash[v - p->start[p->pivot]] = mismatch_found_hash;
			mm_marked[v - p->start[p->pivot]] = TRUE;
		}
}

void mark_old_mismatch ( Partition *p, uint8_t *mm_marked, Info_Vertex *info_vert, uint16_t mmv )
{
	const MismatchCollection *mm = p->mm_collection;
	uint16_t v;
	uint16_t hash;

	for ( v = p->start[p->pivot]; info_vert[p->vertices[v]].belongs_to != info_vert[p->vertices[mmv]].belongs_to; v++ );
	hash = mm->mismatch_hash[v - p->start[p->pivot]];
	for ( v = mmv; v < p->start[p->pivot+1]; v++ )
		if ( ( info_vert[p->vertices[mmv]].belongs_to == info_vert[p->vertices[v]].belongs_to ) && !mm_marked[v - p->start[p->pivot]] )
		{
			mm->mismatch_hash[v - p->start[p->pivot]] = hash;
			mm_marked[v - p->start[p->pivot]] = TRUE;
		}
}

uint16_t get_most_repeated_mismatch_vertex ( MismatchCollection *mmc )
{
	const uint16_t nom = mmc->nom;
	uint16_t original[nom];
	uint16_t i;
	uint16_t count, best_count;
	uint16_t most_repeated_hash;

	memcpy ( original, mmc->mismatch_hash, nom * sizeof ( uint16_t ) );
	sort_uint16 ( nom, mmc->mismatch_hash );
	most_repeated_hash = mmc->mismatch_hash[0];
	count = best_count = 1;
	for ( i = 1; (i < nom) && (mmc->mismatch_hash[i] != 0); i++ )
	{
		if ( mmc->mismatch_hash[i-1] == mmc->mismatch_hash[i] )
		{
			count++;
		}
		else
		{
			if ( count > best_count )
				most_repeated_hash = mmc->mismatch_hash[i-1];
			count = 1;
		}
	}
	for ( i = 0; i < nom; i++ )
		if ( original[i] == most_repeated_hash )
			return i;
	return 0;
}

void compute_mismatches ( MismatchCollection *mmc )
{
	const uint16_t nom = mmc->nom;
	uint16_t i;

	sort_uint16 ( nom, mmc->mismatch_hash );
	mmc->nom = 1;
	mmc->counters[0] = 1;
	for ( i = 1; (i < nom) && (mmc->mismatch_hash[i] != 0); i++ )
	{
		if ( mmc->mismatch_hash[mmc->nom-1] != mmc->mismatch_hash[i] )
		{
			mmc->mismatch_hash[mmc->nom] = mmc->mismatch_hash[i];
			mmc->counters[mmc->nom++] = 1;
		}
		else
			mmc->counters[mmc->nom-1]++;
	}
}

uint8_t mismatch_in_collection ( MismatchCollection *mmc, uint16_t *counters, uint16_t orbit_size )
{
	int middle, sup, inf;
	uint8_t found;

	found = FALSE;
	inf = 0;
	sup = mmc->nom-1;
	do
	{
		middle = ( sup - inf ) / 2 + inf;
		found = ( mmc->mismatch_hash[middle] == mismatch_found_hash );
		if ( !found )
/*			(mismatch_found_hash > mmc->mismatch_hash[middle])?(inf=middle+1):(sup=middle-1); #due to sort in an invert way, we use the line below */
			(mismatch_found_hash < mmc->mismatch_hash[middle])?(inf=middle+1):(sup=middle-1);
		else
			counters[middle] = (uint16_t)(counters[middle] + orbit_size);
	} while ( inf <= sup && !found );
	return found && ( counters[middle] <= mmc->counters[middle] );
}

