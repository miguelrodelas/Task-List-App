NULL = 
SUBDIRS = \
        $(NULL)

INCLUDES =				\
	$(COUCHDB_GLIB_CFLAGS)		\
	$(DESKTOPCOUCH_GLIB_CFLAGS)	\
	-I$(top_srcdir)/couchdb-glib	\
	-I$(top_srcdir)/desktopcouch-glib

test_list_databases_SOURCES = test-list-databases.c
test_list_databases_LDADD = 	\
	$(COUCHDB_GLIB_LIBS)	\
	$(OAUTH_LIBS)		\
	-luuid			\
	$(top_builddir)/couchdb-glib/libcouchdb-glib-1.0.la

test_couchdb_glib_SOURCES = test-couchdb-glib.c
test_couchdb_glib_LDADD = 	\
	$(COUCHDB_GLIB_LIBS)	\
	$(OAUTH_LIBS)		\
	-luuid			\
	$(top_builddir)/couchdb-glib/libcouchdb-glib-1.0.la

test_desktopcouch_glib_SOURCES = test-desktopcouch-glib.c
test_desktopcouch_glib_LDADD =       				\
	$(COUCHDB_GLIB_LIBS)    				\
	$(DESKTOPCOUCH_GLIB_LIBS)				\
	$(OAUTH_LIBS)           				\
	-luuid                  				\
	$(top_builddir)/couchdb-glib/libcouchdb-glib-1.0.la	\
	$(top_builddir)/desktopcouch-glib/libdesktopcouch-glib-1.0.la

test_oauth_SOURCES = test-oauth.c
test_oauth_LDADD = 	\
	$(COUCHDB_GLIB_LIBS)	\
	$(OAUTH_LIBS)		\
	-luuid			\
	$(top_builddir)/couchdb-glib/libcouchdb-glib-1.0.la

EXTRA_DIST = createCouchContacts.py test-oauth.py

check_PROGRAMS = \
	test-couchdb-glib \
	test-desktopcouch-glib \
	$(NULL)

noinst_PROGRAMS = \
	$(check_PROGRAMS) \
	test-list-databases	\
	test-oauth \
	$(NULL)

COUCHDB_VAR = $(top_srcdir)/tests/var
COUCHDB_PID = $(top_srcdir)/tests/config/couchdb.pid
COUCHDB_STDOUT = $(top_srcdir)/tests/var/couchdb.stdout
COUCHDB_STDERR = $(top_srcdir)/tests/var/couchdb.stderr
COUCHDB_CONFIG_AUTH = $(top_srcdir)/tests/config/couchdb-auth.ini
REPORT_XML = $(top_srcdir)/tests/report.xml
REPORT_HTML = $(top_srcdir)/tests/report.html

stop-couchdb:
	@/usr/bin/couchdb -p $(COUCHDB_PID) -d
	@/bin/rm -f $(COUCHDB_PID)

list:
	@/usr/bin/gtester -q -l -p="$(TESTPATH)" $(check_PROGRAMS)

test: $(check_PROGRAMS)
	@echo "Setting up the couchdb server..."
	@/usr/bin/couchdb -p $(COUCHDB_PID) -d > /dev/null
	@/bin/rm -rf $(COUCHDB_VAR)
	@/bin/mkdir -p $(COUCHDB_VAR)
	@/usr/bin/couchdb -n -a /etc/couchdb/default.ini \
	    -o $(COUCHDB_STDOUT) -e $(COUCHDB_STDERR) \
	    -a $(COUCHDB_CONFIG_AUTH) -p $(COUCHDB_PID) -b > /dev/null
	@/bin/sleep 1
	-@/usr/bin/gtester -q -k -o $(REPORT_XML) -p="$(TESTPATH)" \
	    $(check_PROGRAMS) > /dev/null
	-@/usr/bin/gtester-report $(REPORT_XML) > $(REPORT_HTML)
	@keep_couchdb="$(KEEP_COUCHDB)x"; \
    if test $$keep_couchdb == "x"; then \
	    echo "Tearing down the couchdb server..."; \
	    /usr/bin/couchdb -p $(COUCHDB_PID) -d > /dev/null; \
	fi
