CXX = g++
CXXFLAGS = -std=c++17 -Wall

all: build/server build/client

build/server: main.cpp utils.cpp utils.hpp buffer.cpp buffer.hpp hashtable.cpp hashtable.hpp | build
	$(CXX) $(CXXFLAGS) main.cpp utils.cpp buffer.cpp hashtable.cpp -o build/server

build/client: client.cpp utils.cpp utils.hpp buffer.cpp buffer.hpp hashtable.cpp hashtable.hpp | build
	$(CXX) $(CXXFLAGS) client.cpp utils.cpp buffer.cpp hashtable.cpp -o build/client

build:
	mkdir -p build

clean:
	rm -rf build