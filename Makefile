CXX:=g++ #-8
# CXXFLAGS := -std=c++17 -Wall -Werror -pedantic
CXXFLAGS := -std=c++17 -Weffc++  -Wctor-dtor-privacy -Wpedantic -Wall -Wextra -Wconversion -Wsign-conversion  -Wstrict-null-sentinel -Wold-style-cast -Wnoexcept  -Woverloaded-virtual -Wsign-promo -Wzero-as-null-pointer-constant -Wsuggest-final-types -Wsuggest-final-methods -Wsuggest-override -fconcepts -lquadmath
# ggdb3 long128
LDFLAGS:=-lpthread -ldl

all: robin packetsender

rebuild : clean all

robin: robin.cpp loguru.o TrackingServer.o Tracker.o
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)


packetsender: packetsender.cpp
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	#clear; rm -rf *.o *.log; rm -rf robin packetsender
        find . -type f -name "*.o" -name "*.log" ! -name "loguru.o" --delete; rm -rf robin packetsender
