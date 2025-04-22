CXX := g++
CXXFLAGS := -O3 -std=c++20 -Wall

SRCS := main.cc bfcpu.cc
TARGET := bfcpu

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS)

.PHONY: all clean
clean:
	rm -f $(TARGET)
