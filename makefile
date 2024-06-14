CC=gcc
CXX=g++
LD=gcc
CFLAGS=-static -fPIC -std=c++20 -O3 -I. -Iinclude
CXXFLAGS=$(CFLAGS) -fmax-errors=5
LDFLAGS=-s -g
LDLIBS=-lstdc++ -lm

OBJS=
OBJS+=test.o
OBJS+=tiffhdr.o
OBJS+=unibmp.o

all: ubtest

ubtest: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -f ubtest

