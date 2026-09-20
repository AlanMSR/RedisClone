CXX = g++
CXXFLAGS = -std=c++17 -Wall

SRCS = $(wildcard *.cpp)
SERVER_SRCS = $(filter-out client.cpp,$(SRCS))
CLIENT_SRCS = $(filter-out main.cpp,$(SRCS))

.PHONY: all clean

all: build/server build/client

build/server: $(SERVER_SRCS) | build
	$(CXX) $(CXXFLAGS) $(SERVER_SRCS) -o $@

build/client: $(CLIENT_SRCS) | build
	$(CXX) $(CXXFLAGS) $(CLIENT_SRCS) -o $@

build:
	mkdir -p build

clean:
	rm -rf build