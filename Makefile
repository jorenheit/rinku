# Makefile for installing Rinku

prefix      ?= /usr/local
includedir  ?= $(prefix)/include
libdir      ?= $(prefix)/lib

SRC_DIRS    := rinku rinku/util rinku/logic
DEST_SUBDIRS:= $(SRC_DIRS)

CXX         ?= g++
CC          ?= gcc
CXXFLAGS    ?= -std=c++20 -O3
CFLAGS      ?= -O3
AR          := ar
ARFLAGS     := rcs

.PHONY: all install uninstall debug install-debug clean

install:
	@echo "Installing headers to $(DESTDIR)$(includedir)"
	@for src in $(SRC_DIRS); do \
	  mkdir -p "$(DESTDIR)$(includedir)/$$src"; \
	  find $$src -maxdepth 1 -type f \( -name '*.h' -o -name '*.inl' \) -print \
	    | xargs -I{} install -m644 "{}" "$(DESTDIR)$(includedir)/$$src/"; \
	done
	@echo "Done."

uninstall:
	@echo "Removing installed headers from $(DESTDIR)$(includedir)"
	@for sub in $(DEST_SUBDIRS); do \
	  rm -rf "$(DESTDIR)$(includedir)/$$sub"; \
	done
	@echo "Done."

# ---------------------------------------------------------------------------
# Debugger support: compile linenoise.c + rinku_debug.cc into a static lib
# ---------------------------------------------------------------------------

DEBUG_OBJ      := linenoise.o rinku_debug.o
DEBUG_LIB      := librinku.a
LN_SRC         := rinku/linenoise/linenoise.c
DBG_SRC        := rinku/rinku_debug.cc


debug: $(DEBUG_LIB) install-debug
	@echo "Debugger support built and installed."

$(DEBUG_LIB): $(DEBUG_OBJ)
	$(AR) $(ARFLAGS) $@ $^

linenoise.o: $(LN_SRC)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

rinku_debug.o: $(DBG_SRC)
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

install-debug: $(DEBUG_LIB)
	@echo "Installing debugger static library ..."
	@install -d "$(DESTDIR)$(libdir)"
	@install -m644 -v $(DEBUG_LIB) "$(DESTDIR)$(libdir)/"
	@echo "Done."

uninstall-debug:
	@echo "Removing debugger static library ..."
	@rm -f "$(DESTDIR)$(libdir)/$(DEBUG_LIB)"
	@echo "Done."

clean:
	@echo "Cleaning up build artifacts ..."
	@rm -f *.o *.a
	@echo "Done."
