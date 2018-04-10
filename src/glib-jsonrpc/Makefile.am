SUBDIRS = json-glib
noinst_LIBRARIES = libglibjsonrpc_local.a

libglibjsonrpc_local_a_CPPFLAGS = \
	-DPREFIX=\""$(prefix)"\" \
	-DLIBDIR=\""$(libdir)"\" \
	-DJSON_LOCALEDIR=\""$(localedir)"\" \
	-DJSON_COMPILATION=1 \
	-DG_LOG_DOMAIN=\"Json\" \
	-I$(top_srcdir)/src/glib-jsonrpc \
        $(GTK_CFLAGS)

libglibjsonrpc_local_a_SOURCES = \
  glib-jsonrpc-json.c \
  glib-jsonrpc-server.c \
  glib-jsonrpc-client.c

libglibjsonrpc_local_a_incdir = 
libglibjsonrpc_local_a_inc_HEADERS = \
  glib-jsonrpc-json.h \
  glib-jsonrpc-server.h \
  glib-jsonrpc-client.h 

# set the include path found by configure
AM_CPPFLAGS= $(all_includes)

EXTRA_DIST = SConscript
