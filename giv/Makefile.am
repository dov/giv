bin_PROGRAMS = giv

EXTRA_DIST = makefile.mingw giv.nsi giv_types.h giv_mark_tree.h colormaps.h svg.c SConstruct giv.nsi.in giv.rc.in giv-logo.ico 

giv_SOURCES = giv.c giv-backstore.c giv_mark_tree.c colormaps.c svg.c giv-logo.i
giv_LDADD  =  $(GTK_LIBS) 
giv_LDFLAGS = @MY_DEP_LIBS@
giv_CFLAGS = -g
noinst_HEADERS = giv.h giv_types.h giv-backstore.h svg.h

giv-logo.i: ../doc/giv-logo.png
	gdk-pixbuf-csource --name=image_giv_icon_inline ../doc/giv-logo.png > $@

INCLUDES = 	@MY_DEP_CFLAGS@


