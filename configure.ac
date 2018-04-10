AC_INIT(README)

AM_CONFIG_HEADER(config.h)

PACKAGE=givwidget
GIVWIDGET_API_VERSION=2.0

AM_INIT_AUTOMAKE(giv, 0.9.30)

dnl Use libtool to get shared libraries
LT_PREREQ
LT_INIT
AC_DISABLE_STATIC

dnl dito install
AC_PROG_INSTALL
AM_MAINTAINER_MODE

AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_CXX
AC_ISC_POSIX
AC_HEADER_STDC
AC_CHECK_HEADERS([/usr/include/cfitsio/fitsio.h],[CFITSIOINC=-I/usr/include/cfitsio])
AM_PROG_CC_C_O
GIT_COMMIT_ID=`git rev-parse HEAD`
AC_DEFINE_UNQUOTED([GIT_COMMIT_ID],["$GIT_COMMIT_ID"],[Git commit id])
GIT_COMMIT_TIME=`git log --pretty=\'%ci\' -n1`
AC_DEFINE_UNQUOTED([GIT_COMMIT_TIME],"[$GIT_COMMIT_TIME"],[Git commit time])
SYS=`uname`
ARCH=`arch`
AC_DEFINE_UNQUOTED([ARCH],["$SYS $ARCH"],["arch"])

GTK_REQUIRED_VERSION=3.0.0
REQ_PACKAGES="gtk+-3.0"

AM_PATH_GTK_3_0($GTK_REQUIRED_VERSION, :,
  AC_MSG_ERROR([
*** GLIB $GLIB_REQUIRED_VERSION or better is required. The latest version of 
*** GLIB is always available from ftp://ftp.gtk.org/.]),
  gobject gmodule gthread)

LIBS="$LIBS $GTK_LIBS -lcfitsio"

MY_DEP_CFLAGS="`$PKG_CONFIG --cflags  $REQ_PACKAGES`"
MY_DEP_LIBS="`$PKG_CONFIG --libs  $REQ_PACKAGES`"
AC_CHECK_LIB(cfitsio,ffopen)
AC_SUBST(MY_DEP_CFLAGS)
AC_SUBST(MY_DEP_LIBS)
AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST([CFITSIOINC])

dnl AC_OUTPUT(Makefile src/Makefile src/plis/Makefile src/agg/Makefile src/gtkimageviewer/Makefile src/plugins/Makefile examples/Makefile doc/Makefile src/pcre/Makefile src/plugins/dcmtk/Makefile src/plugins/dcmtk/config/Makefile src/plugins/dcmtk/config/include/Makefile src/plugins/dcmtk/ofstd/Makefile src/plugins/dcmtk/ofstd/libsrc/Makefile src/plugins/dcmtk/ofstd/include/Makefile src/plugins/dcmtk/ofstd/include/dcmtk/Makefile src/plugins/dcmtk/ofstd/include/dcmtk/ofstd/Makefile src/plugins/dcmtk/dcmdata/Makefile src/plugins/dcmtk/dcmdata/libsrc/Makefile src/plugins/dcmtk/dcmdata/include/Makefile src/plugins/dcmtk/dcmdata/include/dcmtk/Makefile src/plugins/dcmtk/dcmdata/include/dcmtk/dcmdata/Makefile )

AC_OUTPUT(Makefile src/Makefile src/plis/Makefile src/agg/Makefile src/gtkimageviewer/Makefile src/plugins/Makefile examples/Makefile doc/Makefile src/pcre/Makefile src/glib-jsonrpc/Makefile src/glib-jsonrpc/json-glib/Makefile )


