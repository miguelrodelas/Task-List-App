# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.52)

AC_INIT(evolution-couchdb, 0.4.90, https://bugs.launchpad.net/bugs/+filebug)
AC_CONFIG_SRCDIR(addressbook)
AC_CONFIG_SRCDIR(calendar)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AM_CONFIG_HEADER(config.h)

dnl Initialize maintainer mode
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

dnl Initialize libtool
AM_PROG_LIBTOOL

PKG_PROG_PKG_CONFIG

dnl i18n
IT_PROG_INTLTOOL([0.35.5])
AM_GLIB_GNU_GETTEXT

GETTEXT_PACKAGE=evolution-couchdb
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Evolution UbuntuOne backend])

localedir='$(prefix)/$(DATADIRNAME)/locale'
AC_SUBST(localedir)

dnl Check for dependencies
PKG_CHECK_MODULES(EVOLUTION, glib-2.0 couchdb-glib-1.0 >= 0.6.3 desktopcouch-glib-1.0 >= 0.6.3 libebook-1.2 libedata-book-1.2 dbus-glib-1 gnome-keyring-1)
AC_SUBST(EVOLUTION_CFLAGS)
AC_SUBST(EVOLUTION_LIBS)

PKG_CHECK_MODULES(EPLUGIN, gtk+-2.0 evolution-plugin)
AC_SUBST(EPLUGIN_CFLAGS)
AC_SUBST(EPLUGIN_LIBS)

EDS_API_VERSION=1.2
PKG_CHECK_EXISTS(evolution-data-server-1.2,
	EDS_EXTENSION_DIR=`$PKG_CONFIG --variable=extensiondir evolution-data-server-1.2`,
	AC_MSG_ERROR(You need evolution-data-server to compile evolution-couchdb))				    
AC_SUBST(EDS_API_VERSION)
AC_SUBST(EDS_EXTENSION_DIR)

PKG_CHECK_EXISTS(evolution-plugin,
	EVOLUTION_PLUGINS_DIR=`$PKG_CONFIG --variable=plugindir evolution-plugin`,
	AC_MSG_ERROR(You need evolution-plugin to compile evolution-couchdb))
AC_SUBST(EVOLUTION_PLUGINS_DIR)

dnl Makefiles
AC_OUTPUT([
Makefile
addressbook/Makefile
addressbook/GNOME_Evolution_CouchDB.server
calendar/Makefile
calendar/GNOME_Evolution_CouchDB.server
plugins/Makefile
plugins/org-gnome-evolution-couchdb.eplug.xml
po/Makefile.in
])
