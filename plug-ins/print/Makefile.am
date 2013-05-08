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
print_RC = print.rc.o
endif

AM_LDFLAGS = $(mwindows)

INCLUDES = \
	-I$(top_srcdir)		\
	$(GTK_CFLAGS)		\
	$(GEGL_CFLAGS) \
	-I$(includedir)

LDADD = \
	$(libpicmanui)		\
	$(libpicmanwidgets)	\
	$(libpicmanconfig)	\
	$(libpicman)		\
	$(libpicmancolor)		\
	$(libpicmanmath)		\
	$(libpicmanbase)		\
	$(GTK_LIBS)		\
	$(GEGL_LIBS)		\
	$(RT_LIBS)		\
	$(INTLLIBS)		\
	$(print_RC)

libexecdir = $(picmanplugindir)/plug-ins

libexec_PROGRAMS = print

EXTRA_PROGRAMS = print

print_SOURCES = \
	print.c			\
	print.h			\
	print-draw-page.c	\
	print-draw-page.h	\
	print-page-layout.c	\
	print-page-layout.h	\
	print-page-setup.h	\
	print-page-setup.c	\
	print-preview.c		\
	print-preview.h		\
	print-settings.c	\
	print-settings.h	\
	print-utils.c		\
	print-utils.h
