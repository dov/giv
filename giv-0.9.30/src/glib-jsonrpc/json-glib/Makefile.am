noinst_LIBRARIES = libjsonglib_local.a

# main library
source_h = \
	json-builder.h		\
	json-generator.h 	\
	json-gobject.h 		\
	json-gvariant.h		\
	json-parser.h 		\
	json-path.h		\
	json-reader.h		\
	json-types.h 		\
	json-utils.h		\
	json-version-macros.h	\
	$(NULL)

source_h_private = \
	json-debug.h		\
	json-gobject-private.h	\
	json-scanner.h 		\
	json-types-private.h	\
	$(NULL)

source_c = \
	json-array.c 		\
	json-builder.c	\
	json-debug.c		\
	json-gboxed.c		\
	json-generator.c 	\
	json-gobject.c 	\
	json-gvariant.c	\
	json-node.c 		\
	json-object.c 	\
	json-parser.c 	\
	json-path.c		\
	json-reader.c		\
	json-scanner.c 	\
	json-serializable.c	\
	json-utils.c		\
	json-value.c		\
	$(NULL)

libjsonglib_local_a_CPPFLAGS = \
	-DPREFIX=\""$(prefix)"\" \
	-DLIBDIR=\""$(libdir)"\" \
	-DJSON_LOCALEDIR=\""$(localedir)"\" \
	-DJSON_COMPILATION=1 \
	-DG_LOG_DOMAIN=\"Json\" \
	-I$(top_srcdir)/src/glib-jsonrpc \
        $(GTK_CFLAGS)

libjsonglib_local_a_SOURCES = $(source_c) $(source_h) $(source_h_private) $(BUILT_SOURCES)

libjsonglib_local_a_incdir = 

libjsonglib_local_a_inc_HEADERS = \
           json-builder.h \
           json-debug.h \
           json-enum-types.h \
           json-generator.h \
           json-glib.h \
           json-gobject.h \
           json-gobject-private.h \
           json-gvariant.h \
           json-marshal.h \
           json-parser.h \
           json-path.h \
           json-reader.h \
           json-scanner.h \
           json-types.h \
           json-types-private.h \
           json-version-macros.h \
           json-version.h 

AM_CPPFLAGS= $(all_includes)
EXTRA_DIST = SConscript
