BIN_PATH=../bin

EXE = $(BIN_PATH)/conauto-2.02
OFLAGS = -O2

SOURCES=graph.c \
	groups.c \
	orbits.c \
	refinements.c \
	seqpart.c \
	mismatches.c \
	sort.c	\
	factor.c \
	conauto.c

CC = gcc
CFLAGS = -Wall -Wconversion -Wredundant-decls -Wunreachable-code -Wuninitialized -Wformat=2

# AGC: automorphism group computation. Should be defined always
# PCS: pivot cell selection
# EAD: early automorphism detection
# BKJ: backjumping
# RF: record failure
# PGO: partial graph orbits

CFLAGS += -DAGC -DPGO
#CFLAGS += -DAGC -DPCS -DEAD -DBKJ -DRF -DPGO

ifndef VFLAGS
VFLAGS=-DPCS -DEAD -DBKJ -DRF
endif

OBJS=$(SOURCES:.c=.o)

all: conauto-2.03

conauto-2.0%: $(OBJS)
	$(CC) -o $(BIN_PATH)/$@ $(OBJS)

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(OFLAGS) $(VFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJS)

bin-clean:
	rm -f $(BIN_PATH)/*
