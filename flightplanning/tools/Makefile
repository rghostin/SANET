CXX:=g++ #-8
CXXFLAGS := -std=c++17 -Weffc++  -Wctor-dtor-privacy -Wpedantic -Wall -Wextra -Wconversion -Wsign-conversion  -Wstrict-null-sentinel -Wold-style-cast -Wnoexcept  -Woverloaded-virtual -Wsign-promo -Wzero-as-null-pointer-constant -Wsuggest-final-types -Wsuggest-final-methods -Wsuggest-override -fconcepts -lquadmath
LDFLAGS:=-lpthread -ldl

all: nodemapsender

rebuild : clean all

nodemapsender: nodemapsender.cpp
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf *.o nodemapsender usocket

