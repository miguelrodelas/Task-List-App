AC_INIT([couchdb-glib], [0.6.3], [], [couchdb-glib])

AC_PREREQ([2.59])

AC_CONFIG_SRCDIR([Makefile.am])

AC_CONFIG_MACRO_DIR([m4])

AM_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([1.6.3 foreign])

AC_GNU_SOURCE
AC_ENABLE_SHARED
AC_DISABLE_STATIC

AC_PROG_CC
AC_PROG_LIBTOOL

dnl glib-genmarshal
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

AC_ARG_ENABLE([debug-messages],
    AS_HELP_STRING([--disable-debug-messages],[disable all debug messages]))

AS_IF([test "x$enable_debug_messages" != "xno"], [
    AC_DEFINE(DEBUG_MESSAGES, 1, [Define if debug messages are enabled.])
])


dnl Check for oAuth dependencies
have_oauth="no"
AC_CHECK_HEADERS(unistd.h time.h string.h alloca.h stdio.h stdarg.h math.h openssl/hmac.h, have_oauth_headers="yes", have_oauth_headers="no")
AC_CHECK_LIB(crypto, CRYPTO_free, have_oauth_libs="yes", have_oauth_libs="no")
if test "x$have_oauth_headers" = "xyes" -a "x$have_oauth_libs" = "xyes"; then
   AC_DEFINE(HAVE_OAUTH, 1, [Define to build OAUTH signing code])
   have_oauth="yes"
   OAUTH_LIBS="-lm -lcrypto"
   AC_SUBST(OAUTH_LIBS)
fi
AM_CONDITIONAL(HAVE_OAUTH, test "x$have_oauth" = "xyes")

dnl Look for needed modules
PKG_CHECK_MODULES(COUCHDB_GLIB, glib-2.0 gobject-2.0 json-glib-1.0 >= 0.7.4 libsoup-2.4 libsoup-gnome-2.4 uuid)
AC_SUBST(COUCHDB_GLIB_CFLAGS)
AC_SUBST(COUCHDB_GLIB_LIBS)

dnl Needed modules for Desktopcouch
PKG_CHECK_MODULES(DESKTOPCOUCH_GLIB, gnome-keyring-1 dbus-glib-1)
AC_SUBST(DESKTOPCOUCH_GLIB_CFLAGS)
AC_SUBST(DESKTOPCOUCH_GLIB_LIBS)

LIBCOUCHDBGLIB_CURRENT=2
LIBCOUCHDBGLIB_REVISION=0
LIBCOUCHDBGLIB_AGE=0
AC_SUBST(LIBCOUCHDBGLIB_CURRENT)
AC_SUBST(LIBCOUCHDBGLIB_REVISION)
AC_SUBST(LIBCOUCHDBGLIB_AGE)

##################################################
# Check for gtk-doc.
##################################################
GTK_DOC_CHECK(1.0)
DISTCHECK_CONFIGURE_FLAGS="--enable-gtk-doc"
AC_SUBST(DISTCHECK_CONFIGURE_FLAGS)

##################################################
# Check for introspection
##################################################
GOBJECT_INTROSPECTION_CHECK([0.6.4])

AC_OUTPUT([
Makefile
couchdb-glib.pc
desktopcouch-glib.pc
couchdb-glib/Makefile
desktopcouch-glib/Makefile
tests/Makefile
doc/Makefile
doc/reference/Makefile
])

AC_MSG_NOTICE([

 couchdb-glib configured:
 ------------------------

 version: $VERSION
 oAuth:   $have_oauth

])
