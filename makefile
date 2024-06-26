CC=gcc
CXX=g++
LD=gcc
AR=ar
RANLIB=ranlib
CFLAGS=-static -fPIC -fopenmp -std=c++20 -O3 -I. -Iinclude
CXXFLAGS=$(CFLAGS) -fmax-errors=5
LDFLAGS=-s -g -L. -fopenmp
LDLIBS=-lstdc++ -lm -lgomp -lunibmp

OBJS=
OBJS+=tiffhdr.o
OBJS+=unibmp.o
OBJS+=gifldr.o
OBJS+=ImageAnim.o
OBJS+=PaletteGen.o

all: ubtest libunibmp.a

libunibmp.a: $(OBJS)
	$(AR) qc $@ $^
	$(RANLIB) $@

libunibmp.so: libunibmp.a
	$(LD) -shared $(LDFLAGS) $^ -o $@ $(LDLIBS)

ubtest: test.o libunibmp.a
	$(LD) $(LDFLAGS) $^ -o $@ $(LDLIBS)

test:
	./ubtest

clean:
	rm -f ubtest *.a *.o *.so

