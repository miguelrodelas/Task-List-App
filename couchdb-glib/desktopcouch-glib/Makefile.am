INCLUDES =				\
	-I$(top_srcdir)/couchdb-glib	\
	$(COUCHDB_GLIB_CFLAGS)		\
	$(DESKTOPCOUCH_GLIB_CFLAGS)

lib_LTLIBRARIES = libdesktopcouch-glib-1.0.la

libdesktopcouch_glib_1_0_la_headers =	\
	desktopcouch-document.h		\
	desktopcouch-document-contact.h	\
	desktopcouch-document-task.h	\
	desktopcouch-glib.h		\
	desktopcouch-session.h

libdesktopcouch_glib_1_0_la_SOURCES =		\
	$(libdesktopcouch_glib_1_0_la_headers) 	\
	desktopcouch-document.c			\
	desktopcouch-document-contact.c		\
	desktopcouch-document-task.c		\
	desktopcouch-session.c

libdesktopcouch_glib_1_0_la_LIBADD =				\
	$(top_builddir)/couchdb-glib/libcouchdb-glib-1.0.la	\
	$(COUCHDB_GLIB_LIBS)					\
	$(DESKTOPCOUCH_GLIB_LIBS)

libdesktopcouch_glib_1_0_la_LDFLAGS =	\
	-version-info $(LIBCOUCHDBGLIB_CURRENT):$(LIBCOUCHDBGLIB_REVISION):$(LIBCOUCHDBGLIB_AGE)

hdir = $(includedir)/desktopcouch-glib-1.0
h_DATA = 				\
	desktopcouch-document.h		\
	desktopcouch-document-contact.h	\
	desktopcouch-document-task.h	\
	desktopcouch-glib.h		\
	desktopcouch-session.h

if HAVE_INTROSPECTION

BUILT_SOURCES = Desktopcouch-1.0.gir Desktopcouch-1.0.typelib

girdir = $(datadir)/gir-1.0
gir_DATA = Desktopcouch-1.0.gir

typelibsdir = $(libdir)/girepository-1.0/
typelibs_DATA = Desktopcouch-1.0.typelib

Desktopcouch-1.0.gir: libdesktopcouch-glib-1.0.la
	$(INTROSPECTION_SCANNER) -v \
	--namespace Desktopcouch --nsversion=1.0 \
	--add-include-path=$(srcdir) \
	--add-include-path=. \
	--add-include-path=$(top_srcdir)/couchdb-glib \
	--include=GObject-2.0 \
	--include=Json-1.0 \
	--include=Couchdb-1.0 \
	--library=desktopcouch-glib-1.0 \
	--pkg gobject-2.0 \
	--pkg json-glib-1.0 \
	--output $@ \
	-I$(top_srcdir)/couchdb-glib \
	-l$(top_builddir)/couchdb-glib/libcouchdb-glib-1.0.la \
	$(h_DATA) \
	$(libdesktopcouch_glib_1_0_la_headers) \
	$(libdesktopcouch_glib_1_0_la_SOURCES)

Desktopcouch-1.0.typelib: Desktopcouch-1.0.gir
	LD_LIBRARY_PATH=.libs$${LD_LIBRARY_PATH:+:$$LD_LIBRARY_PATH} \
	$(INTROSPECTION_COMPILER) \
	--includedir=$(top_srcdir)/couchdb-glib \
	--includedir=. \
	$(INTROSPECTION_COMPILER_OPTS) $< -o $(@F)

endif
