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
gfig_RC = gfig.rc.o
endif

AM_LDFLAGS = $(mwindows)

SUBDIRS = gfig-examples images

libexecdir = $(picmanplugindir)/plug-ins

libexec_PROGRAMS = gfig

gfig_SOURCES = \
	gfig.c			\
	gfig.h			\
	gfig-arc.c		\
	gfig-arc.h		\
	gfig-bezier.c		\
	gfig-bezier.h		\
	gfig-circle.c		\
	gfig-circle.h		\
	gfig-dialog.c		\
	gfig-dialog.h		\
	gfig-dobject.c		\
	gfig-dobject.h		\
	gfig-ellipse.c		\
	gfig-ellipse.h		\
	gfig-grid.c		\
	gfig-grid.h		\
	gfig-line.c		\
	gfig-line.h		\
	gfig-poly.c		\
	gfig-poly.h		\
	gfig-preview.c		\
	gfig-preview.h		\
	gfig-rectangle.c	\
	gfig-rectangle.h	\
	gfig-spiral.c		\
	gfig-spiral.h		\
	gfig-star.c		\
	gfig-star.h		\
	gfig-stock.c		\
	gfig-stock.h		\
	gfig-style.c		\
	gfig-style.h    	\
	gfig-types.h

EXTRA_DIST = README

INCLUDES = \
	-I$(top_srcdir)	\
	$(GTK_CFLAGS)	\
	$(GEGL_CFLAGS) \
	-I$(includedir)

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
	$(gfig_RC)
