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

#ifndef _SORT_H_
#define _SORT_H_

#include <stdint.h>

#include "orbits.h"

void sort_cell ( uint16_t nmemb, uint16_t *base, uint64_t *attributes );

void sort_uint16 ( uint16_t nmemb, uint16_t *base );

void sort_valid_orbits ( uint16_t nmemb, uint16_t *base, Graph_Orbits *go );

uint32_t hash_Meiyan32 ( const void *key, size_t len );

uint16_t hash_Meiyan16 ( const void *key, size_t len );

#endif
