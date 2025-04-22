CXX := g++
CXXFLAGS := -O3 -std=c++20 -Wall

all: hello

hello: hello.cc
	$(CXX) $(CXXFLAGS) -o $@ $<

.PHONY: clean all
clean:
	rm -f hello
