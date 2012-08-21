OS_NAME = $(shell uname -s)
ifeq (${OS_NAME},Darwin)
    CXX = clang++
    STD = -std=c++11 -stdlib=libc++
	RTLIB =
else
    CXX = g++
    STD = -std=gnu++0x
	RTLIB = -lrt
endif

CXXFLAGS = $(STD) -I./libuv/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CXXFLAGS_DEBUG = $(CXXFLAGS) -g -O0

all: x10.a

x10.a: libuv/uv.a x10.o
	mkdir -p objs
	cd objs; \
	ar x ../libuv/uv.a; \
	cp ../x10.o .; \
	ar rcs x10.a *.o; \
	cp x10.a ../; \
	cd ..; \
	rm -rf objs

libuv/uv.a:
	$(MAKE) -C libuv
    
x10.o:
	$(CXX) $(CXXFLAGS) -o x10.o -c x10.cpp $(RTLIB) -lm -lpthread

clean:
	rm -f libuv/uv.a
	rm -f x10.a
	rm -f x10.o
	rm -rf objs