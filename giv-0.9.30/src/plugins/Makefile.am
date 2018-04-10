#SUBDIRS = dcmtk

EXTRA_DIST = SConscript simple_dicom.c npy.c fits.c
AM_CPPFLAGS = @MY_DEP_CFLAGS@ -I$(top_srcdir)/src/gtkimageviewer $(CFITSIOINC)

simpledicom_ladir = $(prefix)/lib/giv-1.0/plugins
simpledicom_la_LTLIBRARIES = libsimple_dicom.la
libsimple_dicom_la_SOURCES = simple_dicom.c
libsimple_dicom_la_LDFLAGS = -avoid-version -module

npy_ladir = $(prefix)/lib/giv-1.0/plugins
npy_la_LTLIBRARIES = libnpy.la
libnpy_la_SOURCES = npy.c
libnpy_la_LDFLAGS = -avoid-version -module

fits_ladir = $(prefix)/lib/giv-1.0/plugins
fits_la_LTLIBRARIES = libfits.la
libfits_la_SOURCES = fits.c
libfits_la_LDFLAGS = -avoid-version -module


