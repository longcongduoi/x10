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

default: demo/app1

demo/app1: src/x10.o libuv/uv.a demo/app1.cpp
	$(CXX) -o demo/app1 $(INCLUDES) $(CXXFLAGS) $(LDFLAGS) src/x10.o libuv/uv.a demo/app1.cpp
    
src/x10.o: src/x10.cpp $(wildcard include/*.h)
	$(CXX) -c -o src/x10.o $(INCLUDES) $(CXXFLAGS) src/x10.cpp

libuv/uv.a:
	$(MAKE) -C libuv

clean-all: clean
	rm -f libuv/uv.a

clean:
	rm -f src/x10.o
	rm -f demo/app1