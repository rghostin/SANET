CXX:=g++
CXXFLAGS := -std=c++17 -Wall -Werror -pedantic
LDFLAGS:=-lpthread -ldl

all: robin packetsender

rebuild : clean all

robin: robin.cpp Dispatcher.o Tracker.o Position.o loguru.o
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)


packetsender: packetsender.cpp
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	clear; rm -rf *.o; rm -rf robin


