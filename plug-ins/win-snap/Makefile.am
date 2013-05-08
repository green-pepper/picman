## Process this file with automake to produce Makefile.in

libpicmanui = $(top_builddir)/libpicman/libpicmanui-$(PICMAN_API_VERSION).la
libpicmanconfig = $(top_builddir)/libpicmanconfig/libpicmanconfig-$(PICMAN_API_VERSION).la
libpicmanwidgets = $(top_builddir)/libpicmanwidgets/libpicmanwidgets-$(PICMAN_API_VERSION).la
libpicman = $(top_builddir)/libpicman/libpicman-$(PICMAN_API_VERSION).la
libpicmancolor = $(top_builddir)/libpicmancolor/libpicmancolor-$(PICMAN_API_VERSION).la
libpicmanbase = $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la
libpicmanmath = $(top_builddir)/libpicmanmath/libpicmanmath-$(PICMAN_API_VERSION).la

libexecdir = $(picmanplugindir)/plug-ins

libexec_PROGRAMS = win-snap

win_snap_SOURCES = \
	winsnap.c	\
	winsnap.h

win_snap_LDFLAGS = -mwindows

EXTRA_DIST = \
	resource.h	\
	select.cur	\
	small.ico	\
	winsnap.ico	\
	winsnap.rc

INCLUDES = \
	-I$(top_srcdir)		\
	$(GTK_CFLAGS)		\
	$(GEGL_CFLAGS)		\
	-I$(includedir)

LDADD = \
	winsnap-win32res.o	\
	$(libpicmanui)		\
	$(libpicmanwidgets)	\
	$(libpicmanconfig)	\
	$(libpicman)		\
	$(libpicmanmath)		\
	$(libpicmancolor)		\
	$(libpicmanbase)		\
	$(RT_LIBS)		\
	$(GTK_LIBS)		\
	$(INTLLIBS)

winsnap-win32res.o: winsnap.rc select.cur small.ico winsnap.ico
	$(WINDRES) winsnap.rc winsnap-win32res.o
