
oauth_headers = oauth.h xmalloc.h
oauth_sources = oauth.c xmalloc.c

if HAVE_OAUTH
OAUTH_FILES = $(oauth_headers) $(oauth_sources)
else
OAUTH_FILES = 
endif

INCLUDES =		\
	$(COUCHDB_GLIB_CFLAGS)

lib_LTLIBRARIES = libcouchdb-glib-1.0.la

# Marshallers
marshal_headers = couchdb-marshal.h
marshal_sources = couchdb-marshal.c
MARSHAL_FILES = $(marshal_headers) $(marshal_sources)

couchdb-marshal.h: couchdb-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=_couchdb_marshal > $@

couchdb-marshal.c: couchdb-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --body --prefix=_couchdb_marshal > $@

libcouchdb_glib_1_0_la_headers =	\
	couchdb-array-field.h		\
	couchdb-credentials.h		\
	couchdb-database-info.h		\
	couchdb-document.h		\
	couchdb-document-info.h		\
	couchdb-glib.h			\
	couchdb-session.h		\
	couchdb-struct-field.h		\
	couchdb-types.h			\
	dbwatch.h			\
	utils.h

libcouchdb_glib_1_0_la_sources =	\
	couchdb-array-field.c		\
	couchdb-credentials.c		\
	couchdb-database-info.c		\
	couchdb-document.c		\
	couchdb-document-info.c		\
	couchdb-session.c		\
	couchdb-struct-field.c		\
	dbwatch.c			\
	utils.c				\
	$(marshal_sources)		\
	$(oauth_sources)

libcouchdb_glib_1_0_la_SOURCES =	\
	$(libcouchdb_glib_1_0_la_headers) \
	$(libcouchdb_glib_1_0_la_sources) \
	$(marshal_headers)		\
	$(oauth_headers)

libcouchdb_glib_1_0_la_LIBADD =		\
	$(COUCHDB_GLIB_LIBS)		\
	$(OAUTH_LIBS)			\
	-luuid

libcouchdb_glib_1_0_la_LDFLAGS =	\
	-version-info $(LIBCOUCHDBGLIB_CURRENT):$(LIBCOUCHDBGLIB_REVISION):$(LIBCOUCHDBGLIB_AGE)

hdir = $(includedir)/couchdb-glib-1.0
h_DATA = 				\
	couchdb-array-field.h		\
	couchdb-credentials.h		\
	couchdb-database-info.h		\
	couchdb-document.h		\
	couchdb-document-info.h		\
	couchdb-glib.h			\
	couchdb-session.h		\
	couchdb-struct-field.h		\
	couchdb-types.h

EXTRA_DIST = $(h_DATA) couchdb-marshal.list $(MARSHAL_FILES) $(OAUTH_FILES)
BUILT_SOURCES = $(MARSHAL_FILES)
CLEANFILES = $(BUILT_SOURCES)

if HAVE_INTROSPECTION

BUILT_SOURCES += Couchdb-1.0.gir Couchdb-1.0.typelib

girdir = $(datadir)/gir-1.0
gir_DATA = Couchdb-1.0.gir

typelibsdir = $(libdir)/girepository-1.0/
typelibs_DATA = Couchdb-1.0.typelib

Couchdb-1.0.gir: libcouchdb-glib-1.0.la
	$(INTROSPECTION_SCANNER) -v \
        --namespace Couchdb --nsversion=1.0 \
        --add-include-path=$(srcdir) \
        --add-include-path=. \
        --include=GObject-2.0 \
        --include=Json-1.0 \
        --library=couchdb-glib-1.0 \
        --pkg gobject-2.0 \
        --pkg json-glib-1.0 \
        --output $@ \
        -I$(top_srcdir) \
        $(h_DATA) \
        $(libcouchdb_glib_1_0_la_headers) \
        $(libcouchdb_glib_1_0_la_sources)

Couchdb-1.0.typelib: Couchdb-1.0.gir
	LD_LIBRARY_PATH=.libs$${LD_LIBRARY_PATH:+:$$LD_LIBRARY_PATH} \
	$(INTROSPECTION_COMPILER) \
        --includedir=$(srcdir) \
	--includedir=. \
	$(INTROSPECTION_COMPILER_OPTS) $< -o $(@F)

endif
