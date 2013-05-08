## Process this file with automake to produce Makefile.in

if OS_WIN32
mwindows = -mwindows
else
libm = -lm
endif

libpicmanui = $(top_builddir)/libpicman/libpicmanui-$(PICMAN_API_VERSION).la
libpicmanconfig = $(top_builddir)/libpicmanconfig/libpicmanconfig-$(PICMAN_API_VERSION).la
libpicmanwidgets = $(top_builddir)/libpicmanwidgets/libpicmanwidgets-$(PICMAN_API_VERSION).la
libpicman = $(top_builddir)/libpicman/libpicman-$(PICMAN_API_VERSION).la
libpicmancolor = $(top_builddir)/libpicmancolor/libpicmancolor-$(PICMAN_API_VERSION).la
libpicmanbase = $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la
libpicmanmath = $(top_builddir)/libpicmanmath/libpicmanmath-$(PICMAN_API_VERSION).la

if HAVE_WINDRES
include $(top_srcdir)/build/windows/picmanrc-plug-ins.rule
flame_RC = flame.rc.o
endif

AM_LDFLAGS = $(mwindows)

libexecdir = $(picmanplugindir)/plug-ins

libexec_PROGRAMS = flame

flame_SOURCES = \
	cmap.c		\
	cmap.h		\
	flame.c		\
	flame.h		\
	libifs.c	\
	libifs.h	\
	rect.c		\
	rect.h

INCLUDES = \
	-I$(top_srcdir)	\
	$(GTK_CFLAGS)	\
	$(GEGL_CFLAGS) \
	-I$(includedir)

EXTRA_DIST = README

LDADD = \
	$(libm)			\
	$(libpicmanui)		\
	$(libpicmanwidgets)	\
	$(libpicmanconfig)	\
	$(libpicman)		\
	$(libpicmancolor)		\
	$(libpicmanmath)		\
	$(libpicmanbase)		\
	$(GTK_LIBS)		\
	$(RT_LIBS)		\
	$(INTLLIBS)		\
	$(flame_RC)
