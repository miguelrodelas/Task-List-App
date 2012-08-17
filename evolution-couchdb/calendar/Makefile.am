INCLUDES =						\
	$(EVOLUTION_CFLAGS)				\
	-I$(top_srcdir)

extensiondir = $(EDS_EXTENSION_DIR)
extension_LTLIBRARIES = libecalbackendcouchdb.la

libecalbackendcouchdb_la_SOURCES =		\
	e-cal-backend-couchdb-factory.h	\
	e-cal-backend-couchdb-factory.c	\
	e-cal-backend-couchdb.c		\
	e-cal-backend-couchdb.h

libecalbackendcouchdb_la_LIBADD =		\
	$(EVOLUTION_LIBS)

libecalbackendcouchdb_la_LDFLAGS = -module -avoid-version
