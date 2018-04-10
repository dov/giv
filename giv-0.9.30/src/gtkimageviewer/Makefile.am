noinst_LIBRARIES = libgtkimageviewer_local.a

libgtkimageviewer_local_a_SOURCES = gtk-image-viewer-fixed.c

libgtkimageviewer_local_a_incdir = 
libgtkimageviewer_local_a_inc_HEADERS = \
  gtk-image-viewer.h \
  gtk-image-viewer-private.h  
noinst_HEADERS = 

libgtkimageviewer_local_a_incdir = $(includedir)/gtkimageviewer
libgtkimageviewer_local_a_LIBADD = 

AM_CPPFLAGS = \
        $(all_includes) \
	-I$(top_srcdir)/src/gtkimageviewer \
	$(GTK_CFLAGS) 

EXTRA_DIST = gtk-image-viewer.gob SConscript

# How do I make this happen at autoconf/automake time and not at compile time?
%.gob.stamp: %.gob
	gob2 $<
	@touch $@ 
