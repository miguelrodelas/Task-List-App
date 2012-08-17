INCLUDES = 			\
	-I$(top_srcdir)	        \
	$(EVOLUTION_CFLAGS)	\
	$(EPLUGIN_CFLAGS)

%.eplug: %.eplug.xml
	cp $< $@

plugindir = $(EVOLUTION_PLUGINS_DIR)
plugin_DATA = org-gnome-evolution-couchdb.eplug
plugin_LTLIBRARIES = liborg-gnome-evolution-couchdb.la

liborg_gnome_evolution_couchdb_la_SOURCES =        \
	couchdb-contacts-source.h \
	couchdb-contacts-source.c \
	couchdb-tasks-source.h \
	couchdb-tasks-source.c 

liborg_gnome_evolution_couchdb_la_LIBADD = 	\
	$(EVOLUTION_CALENDAR_LIBS)		\
	$(EPLUGIN_LIBS)		

liborg_gnome_evolution_couchdb_la_LDFLAGS = -module -avoid-version $(NO_UNDEFINED)

EXTRA_DIST =				\
	org-gnome-evolution-couchdb.eplug.xml

CLEANFILES = org-gnome-evolution-couchdb.eplug
