OS_NAME = $(shell uname -s)
ifeq (${OS_NAME},Darwin)
    CXX = clang++
    CXXFLAGS_COMP_OS = -std=c++11 -stdlib=libc++
    CXXFLAGS_LINK_OS = -framework CoreServices
else
    CXX = g++
    CXXFLAGS_COMP_OS = -std=gnu++0x -lrt
    CXXFLAGS_LINK_OS = -lrt
endif

INCLUDE = -Ilibuv/include -Iinclude
CXXFLAGS = $(INCLUDE) -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CXXFLAGS_DEBUG = $(CXXFLAGS) -g -O0

all: app1

app1: x10.a app1.cpp
	$(CXX) $(CXXFLAGS_DEBUG) $(CXXFLAGS_COMP_OS) -o app1 app1.cpp $(CXXFLAGS_LINK_OS) -lm -lpthread x10.a

x10.a: libuv/uv.a x10.o
	mkdir -p objs
	cd objs; \
	ar x ../libuv/uv.a; \
	cp ../x10.o .; \
	ar rcs x10.a *.o; \
	cp x10.a ../; \
	cd ..; \
	rm -rf objs
    
x10.o: x10.cpp $(wildcard include/*.h)
	$(CXX) $(CXXFLAGS_DEBUG) $(CXXFLAGS_COMP_OS) -o x10.o -c x10.cpp

libuv/uv.a:
	$(MAKE) -C libuv

clean-all: clean
	rm -f libuv/uv.a

clean:
	rm -f x10.a
	rm -f x10.o
	rm -f app1
	rm -rf objs