# Example project Makefile

CXX        := g++
CXXFLAGS   := -O3 -std=c++20 -Wall

# Common source files (excluding main_*.cc)
COMMON_SRCS := bfcpu.cc \
               microcode/microcode.cc 

# Headers-only main vs. debug main
RUN_MAIN   := main_run.cc
DBG_MAIN   := main_debug.cc

# Output binaries
RUN_EXE    := bfcpu
DBG_EXE    := bfcpu_debug

# Rinku library for debug mode
# (assumes librinku.a is in a standard lib path, or use -L/path/to/lib)
RINKU_LIBS := -lrinku

.PHONY: all run debug clean

all: run debug

# Header-only version
run: $(RUN_EXE)

$(RUN_EXE): $(RUN_MAIN) $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) \
	  -o $@ \
	  $(RUN_MAIN) $(COMMON_SRCS)

# Debug version
debug: $(DBG_EXE)

$(DBG_EXE): $(DBG_MAIN) $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) \
	  -o $@ \
	  $(DBG_MAIN) $(COMMON_SRCS) \
	  $(RINKU_LIBS)

clean:
	@echo "Cleaning…"
	rm -f $(RUN_EXE) $(DBG_EXE)
