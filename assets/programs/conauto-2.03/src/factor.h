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

#ifndef _FACTOR_H_
#define _FACTOR_H_

#include <stdint.h>

#define MAX_PRIME_FACTORS 600

typedef struct {
   uint16_t nof;
   uint16_t prime[MAX_PRIME_FACTORS];
   uint16_t exp[MAX_PRIME_FACTORS];
} FactoredInt;

extern uint16_t gcd ( uint16_t a, uint16_t b );

extern void factMultiply ( const FactoredInt *a, const FactoredInt *b, FactoredInt *r );

extern void factDivide ( const FactoredInt *a, const FactoredInt *b, FactoredInt *r );

extern void factorize_factorial ( uint16_t n, FactoredInt *r );

extern void factorize ( const uint16_t n, FactoredInt *r );

extern int factEqual ( const FactoredInt *a, const FactoredInt *b );

#endif
