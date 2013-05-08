## Process this file with automake to produce Makefile.in

libpicmanui = $(top_builddir)/libpicman/libpicmanui-$(PICMAN_API_VERSION).la
libpicmanconfig = $(top_builddir)/libpicmanconfig/libpicmanconfig-$(PICMAN_API_VERSION).la
libpicmanwidgets = $(top_builddir)/libpicmanwidgets/libpicmanwidgets-$(PICMAN_API_VERSION).la
libpicman = $(top_builddir)/libpicman/libpicman-$(PICMAN_API_VERSION).la
libpicmancolor = $(top_builddir)/libpicmancolor/libpicmancolor-$(PICMAN_API_VERSION).la
libpicmanbase = $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la
libpicmanmath = $(top_builddir)/libpicmanmath/libpicmanmath-$(PICMAN_API_VERSION).la

if OS_WIN32
mwindows = -mwindows
endif

if HAVE_WINDRES
include $(top_srcdir)/build/windows/picmanrc-plug-ins.rule
file_faxg3_RC = file-faxg3.rc.o
endif

AM_LDFLAGS = $(mwindows)

libexecdir = $(picmanplugindir)/plug-ins

libexec_PROGRAMS = file-faxg3

file_faxg3_SOURCES = \
	faxg3.c		\
	g3.c		\
	g3.h

INCLUDES = \
	-I$(top_srcdir)		\
	$(CAIRO_CFLAGS)		\
	$(GDK_PIXBUF_CFLAGS)	\
	$(GEGL_CFLAGS) \
	-I$(includedir)

LDADD = \
	$(libpicman)		\
	$(libpicmanconfig)	\
	$(libpicmancolor)		\
	$(libpicmanbase)		\
	$(libpicmanmath)		\
	$(CAIRO_LIBS)		\
	$(GDK_PIXBUF_LIBS)	\
	$(GEGL_LIBS)		\
	$(RT_LIBS)		\
	$(INTLLIBS)		\
	$(file_faxg3_RC)
