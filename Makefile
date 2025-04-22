## Makefile for installing Rinku headers system-wide

prefix ?= /usr/local
includedir ?= $(prefix)/include

srcdir := rinku
destdir := $(DESTDIR)$(includedir)/rinku

.PHONY: all install uninstall
all:
	@echo "Nothing to build. Use 'make install' to install headers."

install:
	@echo "Installing Rinku headers to $(destdir)"
	cd $(srcdir) && \
	  find . -type d -exec install -d "$(DESTDIR)$(includedir)/rinku/{}" \;
	cd $(srcdir) && \
	  find . -type f \( -name '*.h' -o -name '*.hpp' \) \
	    -exec install -m 644 "{}" "$(DESTDIR)$(includedir)/rinku/{}" \;
	@echo "Done."

uninstall:
	@echo "Removing Rinku headers from $(destdir)"
	rm -rf "$(destdir)"
	@echo "Done."
