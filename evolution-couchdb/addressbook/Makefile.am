INCLUDES =						\
	$(EVOLUTION_CFLAGS)				\
	-I$(top_srcdir)

extensiondir = $(EDS_EXTENSION_DIR)
extension_LTLIBRARIES = libebookbackendcouchdb.la

libebookbackendcouchdb_la_SOURCES =		\
	e-book-backend-couchdb-factory.c	\
	e-book-backend-couchdb.c		\
	e-book-backend-couchdb.h

libebookbackendcouchdb_la_LIBADD =		\
	$(EVOLUTION_LIBS)

libebookbackendcouchdb_la_LDFLAGS = -module -avoid-version
