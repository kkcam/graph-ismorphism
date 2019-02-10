#!/bin/sh

MAKE_OPTS=""
#MAKE_OPTS="clean"

#version 2.00
make $MAKE_OPTS conauto-2.00 VFLAGS=""

#version 2.00+DCS
make $MAKE_OPTS conauto-2.00+DCS VFLAGS="-DPCS"

#version 2.00+EAD
make $MAKE_OPTS conauto-2.00+EAD VFLAGS="-DEAD"

#version 2.00+BJ
make $MAKE_OPTS conauto-2.00+BJ VFLAGS="-DBKJ"

#version 2.00+CDR
make $MAKE_OPTS conauto-2.00+CDR VFLAGS="-DRF"

#version 2.00+DCS+EAD
make $MAKE_OPTS conauto-2.00+DCS+EAD VFLAGS="-DPCS -DEAD"

#version 2.00+DCS+BJ
make $MAKE_OPTS conauto-2.00+DCS+BJ VFLAGS="-DPCS -DBKJ"

##version 2.00+XXX
#make $MAKE_OPTS conauto-2.0X+YYY VFLAGS="-DYYY"

#version 2.03
make $MAKE_OPTS all
