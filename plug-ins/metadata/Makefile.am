## Process this file with automake to produce Makefile.in

libpicmanui = $(top_builddir)/libpicman/libpicmanui-$(PICMAN_API_VERSION).la
libpicmanwidgets = $(top_builddir)/libpicmanwidgets/libpicmanwidgets-$(PICMAN_API_VERSION).la
libpicman = $(top_builddir)/libpicman/libpicman-$(PICMAN_API_VERSION).la
libpicmanconfig = $(top_builddir)/libpicmanconfig/libpicmanconfig-$(PICMAN_API_VERSION).la
libpicmancolor = $(top_builddir)/libpicmancolor/libpicmancolor-$(PICMAN_API_VERSION).la
libpicmanbase = $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la
libpicmanmath = $(top_builddir)/libpicmanmath/libpicmanmath-$(PICMAN_API_VERSION).la

if OS_WIN32
mwindows = -mwindows
endif

if HAVE_WINDRES
include $(top_srcdir)/build/windows/picmanrc-plug-ins.rule
metadata_RC = metadata.rc.o
endif

AM_LDFLAGS = $(mwindows)

libexecdir = $(picmanplugindir)/plug-ins

libexec_PROGRAMS = metadata

metadata_SOURCES = \
	metadata.c		\
	metadata.h		\
	xmp-model.h		\
	xmp-model.c		\
	xmp-parse.h		\
	xmp-parse.c		\
	xmp-encode.h		\
	xmp-encode.c		\
	xmp-schemas.h		\
	xmp-schemas.c		\
	interface.h		\
	interface.c		\
	picmanxmpmodelwidget.h	\
	picmanxmpmodelwidget.c	\
	picmanxmpmodelentry.h	\
	picmanxmpmodelentry.c	\
	picmanxmpmodeltext.h	\
	picmanxmpmodeltext.c	\
	exif-decode.h		\
	exif-decode.c
#	exif-encode.h		\
#	exif-encode.c		\
#	iptc-decode.h		\
#	iptc-decode.c	

noinst_PROGRAMS = xmpdump
xmpdump_SOURCES = \
	xmpdump.c	\
	xmp-schemas.h	\
	xmp-schemas.c	\
	xmp-encode.h	\
	xmp-encode.c	\
	xmp-model.h	\
	xmp-model.c	\
	xmp-parse.h	\
	xmp-parse.c	

INCLUDES = \
	-I$(top_srcdir)	\
	$(GTK_CFLAGS)	\
	$(GEGL_CFLAGS) \
	$(EXIF_CFLAGS)	\
	-I$(includedir)

LDADD = \
	$(libpicmanui)		\
	$(libpicmanwidgets)	\
	$(libpicman)		\
	$(libpicmanmath)		\
	$(libpicmanconfig)	\
	$(libpicmancolor)		\
	$(libpicmanbase)		\
	$(EXIF_LIBS)		\
	$(GTK_LIBS)		\
	$(RT_LIBS)		\
	$(INTLLIBS)		\
	$(metadata_RC)
