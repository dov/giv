prefix=@prefix@
SUBDIRS = plis agg gtkimageviewer plugins pcre

lib_LTLIBRARIES = libgivwidget-1.0.la

libgivwidget_1_0_la_SOURCES = giv-widget.cc \
              colormaps.cc \
              giv-parser.cc \
              GivPainterAgg.cc \
              GivPainterCairo.cc \
              GivPainterCairoPixbuf.cc \
              GivRenderer.cc \
              GivStringArray.c \
              giv-data.cc \
              giv-backstore.cc \
              giv-markup.cc \
              giv_agg_arrowhead.cc \
	      givimage.c \
              givplugin.c \
              givregex.c \
	      giv-contrast.cc \
              giv-histo.cc \
	      giv-color-map-fix.c

libgivwidget_1_0_la_inc_HEADERS = giv-parser.h giv-widget.h giv-data.h
libgivwidget_1_0_la_incdir = $(includedir)/givwidget-1.0
libgivwidget_1_0_la_LIBADD = 
libgivwidget_1_0_la_CPPFLAGS = -DPACKAGE_PLUGIN_DIR='"$(prefix)/lib/giv-1.0/plugins"' 

bin_PROGRAMS = giv

EXTRA_DIST = makefile.mingw giv.nsi giv_types.h giv_mark_tree.h colormaps.h svg.c SConscript giv.nsi.in giv.rc giv-logo.ico logo_150_inline.i giv-logo.i menu-top-xml.i menu-popup-xml.i copyright.i giv-contrast.gob giv-histo.gob giv-info.gob giv-mark-tree-dialog.gob giv-widget.gob giv-window.gob giv-win.gob copyright.markup menu-popup.xml menu-top.xml

BUILT_SOURCES=
DISTCLEANFILES=
giv_SOURCES = \
    giv.cc \
    giv-win.cc \
    giv-info.cc \
    giv-mark-tree-dialog.cc 

giv_LDADD  =  libgivwidget-1.0.la $(GTK_LIBS) -Lplis -lplis -Lagg -lagg -Lgtkimageviewer -lgtkimageviewer_local -lpcre
giv_LDFLAGS = @MY_DEP_LIBS@
giv_CPPFLAGS = -DPACKAGE_DOC_DIR='"$(prefix)/share/doc/giv"' -DPACKAGE_PLUGIN_DIR='"$(prefix)/lib/giv-1.0/plugins"' 

noinst_HEADERS = \
    GivPainter.h \
    GivPainterAgg.h \
    GivPainterAggPriv.h \
    GivPainterCairo.h \
    GivPainterCairoPixbuf.h \
    giv-parser.h \
    GivRenderer.h \
    colormaps.h \
    giv-backstore.h \
    giv-info-private.h \
    giv-info.h \
    giv-mark-tree-dialog-private.h \
    giv-mark-tree-dialog.h \
    giv-widget-private.h \
    giv-widget.h \
    giv-win.h \
    giv-win-private.h \
    giv-data.h \
    giv_mark_tree.h \
    giv_agg_arrowhead.h \
    giv-markup.h \
    GivStringArray.h \
    givimage.h \
    givplugin.h \
    givregex.h \
    giv-color-map.h \
    giv-contrast.h giv-contrast-private.h \
    giv-histo.h giv-histo-private.h \
    giv-win.cc giv-win.h giv-win-private.h

EXTRA_DIST += giv-widget.cc giv-widget.h giv-widget-private.h

EXTRA_DIST += giv-mark-tree-dialog.cc giv-mark-tree-dialog.h giv-mark-tree-dialog-private.h

EXTRA_DIST += giv-info.cc giv-info.h giv-info-private.h
EXTRA_DIST += giv-color-map.vala

giv-logo.i: $(top_srcdir)/doc/giv-logo.png
	gdk-pixbuf-csource --name=image_giv_icon_inline $(top_srcdir)/doc/giv-logo.png > $@

INCLUDES = @MY_DEP_CFLAGS@ -I$(top_srcdir)/src/gtkimageviewer

