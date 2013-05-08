## Process this file with automake to produce Makefile.in

libpicmanhelp = libpicmanhelp.a
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
help_RC = help.rc.o
endif

AM_CPPFLAGS = -DDATADIR=\""$(picmandatadir)"\"

AM_LDFLAGS = $(mwindows)

noinst_LIBRARIES = libpicmanhelp.a

libpicmanhelp_a_SOURCES = \
	picmanhelptypes.h		\
	picmanhelp.c		\
	picmanhelp.h		\
	picmanhelpdomain.c	\
	picmanhelpdomain.h	\
	picmanhelpitem.c		\
	picmanhelpitem.h		\
	picmanhelplocale.c	\
	picmanhelplocale.h	\
	picmanhelpprogress.c	\
	picmanhelpprogress.h	\
	picmanhelpprogress-private.h

libexecdir = $(picmanplugindir)/plug-ins

libexec_PROGRAMS = help

help_SOURCES = help.c

INCLUDES = \
	-I$(top_srcdir)		\
	$(CAIRO_CFLAGS)		\
	$(GIO_CFLAGS)		\
	$(GEGL_CFLAGS) \
	$(GDK_PIXBUF_CFLAGS)	\
	-I$(includedir)

LDADD = \
	$(libpicmanhelp)		\
	$(libpicman)		\
	$(libpicmanconfig)	\
	$(libpicmancolor)		\
	$(libpicmanbase)		\
	$(libpicmanmath)		\
	$(CAIRO_LIBS)		\
	$(GIO_LIBS)		\
	$(GDK_PIXBUF_LIBS)	\
	$(RT_LIBS)		\
	$(INTLLIBS)		\
	$(help_RC)


noinst_PROGRAMS = picman-help-lookup

picman_help_lookup_SOURCES = picman-help-lookup.c

picman_help_lookup_LDADD = \
	$(libpicmanhelp)	\
	$(libpicmanbase)	\
	$(GIO_LIBS)
