bin_PROGRAMS = giv

EXTRA_DIST = makefile.mingw giv.nsi giv_types.h giv_mark_tree.h colormaps.h

giv_SOURCES = giv.c giv_mark_tree.c colormaps.c
giv_LDADD  =  $(GTK_LIBS) 
giv_LDFLAGS = @MY_DEP_LIBS@
giv_CFLAGS = -g

INCLUDES = 	@MY_DEP_CFLAGS@


