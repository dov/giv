noinst_LIBRARIES = libgtkimageviewer_local.a

libgtkimageviewer_local_a_SOURCES = gtk-image-viewer-fixed.c

noinst_HEADERS = gtk-image-viewer.h gtk-image-viewer-private.h

libgtkimageviewer_local_a_incdir = $(includedir)/gtkimageviewer
libgtkimageviewer_local_a_LIBADD = 

# set the include path found by configure
INCLUDES= $(all_includes)

AM_CPPFLAGS = \
	$(GTK_CFLAGS) 

EXTRA_DIST = gtk-image-viewer.gob SConscript

# How do I make this happen at autoconf/automake time and not at compile time?
%.gob.stamp: %.gob
	gob2 $<
	@touch $@ 
