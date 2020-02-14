CXX:=g++ #-8
CXXFLAGS := -std=c++17 -Weffc++  -Wctor-dtor-privacy -Wpedantic -Wall -Wextra -Wconversion -Wsign-conversion  -Wstrict-null-sentinel -Wold-style-cast -Wnoexcept  -Woverloaded-virtual -Wsign-promo -Wzero-as-null-pointer-constant -Wsuggest-final-types -Wsuggest-final-methods -Wsuggest-override -fconcepts -lquadmath

LDFLAGS:=-lpthread -ldl

all: command_sender tpacket_broadcaster nodemap_sender

rebuild : clean all

command_sender: command_sender.cpp
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)

tpacket_broadcaster: tpacket_broadcaster.cpp
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)

nodemap_sender: nodemap_sender.cpp
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	find . \( -type f -name "*.o" -and ! -name "loguru.o" -or -name "*.log" \) -delete; rm -rf command_sender tpacket_broadcaster nodemap_sender
