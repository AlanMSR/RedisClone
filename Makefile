CXX = g++
CXXFLAGS = -std=c++17 -Wall

all: build/server build/client

build/server: main.cpp utils.cpp utils.hpp | build
	$(CXX) $(CXXFLAGS) main.cpp utils.cpp -o build/server

build/client: client.cpp utils.cpp utils.hpp | build
	$(CXX) $(CXXFLAGS) client.cpp utils.cpp -o build/client

build:
	mkdir -p build

clean:
	rm -rf build