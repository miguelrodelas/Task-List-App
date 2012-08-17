ACLOCAL_AMFLAGS = -I m4

SUBDIRS = couchdb-glib desktopcouch-glib tests doc

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

pcfiles = couchdb-glib-1.0.pc desktopcouch-glib-1.0.pc

couchdb-glib-1.0.pc: couchdb-glib.pc
	@cp -f $< $@

desktopcouch-glib-1.0.pc: desktopcouch-glib.pc
	@cp -f $< $@

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig

couchdbdocdir = ${datadir}/doc/couchdb-glib
couchdbdoc_DATA =		\
	README				\
	LICENSE				\
	couchdb-glib.doap	\
	NEWS

EXTRA_DIST =			\
	couchdb-glib.pc.in	\
	desktopcouch-glib.pc.in	\
	$(couchdbdoc_DATA)

dist-hook:
	git log --stat > ChangeLog.in && \
	cp -f ChangeLog.in $(distdir)/ChangeLog && \
	rm -f ChangeLog.in

test: all
	@$(am__cd) tests && \
	    $(MAKE) $(AM_MAKEFLAGS) test

list-tests:
	@$(am__cd) tests && \
	    $(MAKE) $(AM_MAKEFLAGS) list

CLEANFILES = $(pcfiles)
