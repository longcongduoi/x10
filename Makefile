uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

ifdef MSVC
uname_S := MINGW
endif

INCLUDES = -Iinclude -Ilibuv/include
CXXFLAGS = -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -g -O0
LDFLAGS = -lm

ifeq (Darwin,$(uname_S))
    CXX = clang++
    CXXFLAGS += -std=c++11 -stdlib=libc++
    LDFLAGS += -framework CoreServices
endif

ifeq (Linux,$(uname_S))
    CXX = g++
    CXXFLAGS += -std=gnu++0x
    LDFLAGS += -lrt
endif

default: demo/app1 demo/lib1.o

demo/app1: demo/app1.cpp $(wildcard include/*.h) libuv/libuv.a demo/lib1.o
	$(CXX) -o demo/app1 $(INCLUDES) $(CXXFLAGS) $(LDFLAGS) demo/lib1.o libuv/libuv.a demo/app1.cpp

demo/lib1.o: libuv/libuv.a demo/lib1.cpp demo/lib1.h $(wildcard include/*.h)
	$(CXX) -o demo/lib1.o $(INCLUDES) $(CXXFLAGS) -c demo/lib1.cpp
    
libuv/libuv.a:
	$(MAKE) -C libuv

clean-all: clean
	rm -f libuv/libuv.a

clean:
	rm -f demo/lib1
	rm -f demo/app1
