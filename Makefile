CXX:=g++ #-8
# CXXFLAGS := -std=c++17 -Wall -Werror -pedantic
CXXFLAGS := -std=c++17 -Weffc++  -Wctor-dtor-privacy -Wpedantic -Wall -Wextra -Wconversion -Wsign-conversion  -Wstrict-null-sentinel -Wold-style-cast -Wnoexcept  -Woverloaded-virtual -Wsign-promo -Wzero-as-null-pointer-constant -Wsuggest-final-types -Wsuggest-final-methods -Wsuggest-override -fconcepts -lquadmath
# ggdb3 long128
LDFLAGS:=-lpthread -ldl -lssl -lcrypto -lsqlite3

all: robin

rebuild : clean all

robin: robin.cpp loguru.o TrackingServer.o CCServer.o
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	find . \( -type f -name "*.o" -and ! -name "loguru.o" -or -name "*.log" \) -delete; rm -rf robin
