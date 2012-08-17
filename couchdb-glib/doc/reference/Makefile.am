## Process this file with automake to produce Makefile.in

# Dummy targets we need to make distcheck and check pass
test test-report perf-report full-report:
	## Ignore

# automake requirements
AUTOMAKE_OPTIONS = 1.7

# The name of the module
DOC_MODULE=couchdb-glib

# The top-level SGML file. You can change this if you want to.
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.sgml

# The directory containing the source code. Relative to $(srcdir).
DOC_SOURCE_DIR=../..

# Extra options to pass to gtkdoc-scangobj. Not normally needed.
SCANGOBJ_OPTIONS=

# Extra options to supply to gtkdoc-scan.
# Fx --rebuild-types --rebuild-sections
SCAN_OPTIONS=

# Extra options to supply to gtkdoc-mkdb.
MKDB_OPTIONS=--sgml-mode --output-format=xml --ignore-files=trio

# Extra options to supply to gtkdoc-fixref. Not normally needed.
FIXXREF_OPTIONS=

# Used for dependencies. The docs will be rebuilt if any of these change.
HFILE_GLOB=$(top_srcdir)/couchdb-glib/couchdb*.h $(top_srcdir)/desktopcouch-glib/desktopcouch*.h
CFILE_GLOB=$(top_srcdir)/couchdb-glib/couchdb*.c $(top_srcdir)/desktopcouch-glib/desktopcouch*.c

# Header files to ignore when scanning.
IGNORE_HFILES=		\
	xmalloc.h		\
	dbwatch.h		\
	oauth.h			\
	utils.h			\
	xmalloc.h		\
	config.h

# Images to copy into HTML directory.
HTML_IMAGES=

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE).
content_files = 

# CFLAGS and LDFLAGS for compiling gtkdoc-scangobj with your library.
INCLUDES=-I$(top_srcdir)/couchdb-glib   \
         $(COUCHDB_GLIB_CFLAGS)

GTKDOC_LIBS=$(COUCHDB_GLIB_LIBS) \
            $(top_builddir)/couchdb-glib/libcouchdb-glib-1.0.la	\
            $(top_builddir)/desktopcouch-glib/libdesktopcouch-glib-1.0.la

# This includes the standard gtk-doc make rules, copied by gtkdocize.
include $(top_srcdir)/gtk-doc.make

# Other files to distribute
# e.g. EXTRA_DIST += version.xml.in
EXTRA_DIST += \
	couchdb-glib.types        \
	couchdb-glib-docs.sgml
#	couchdb-glib-sections.txt

