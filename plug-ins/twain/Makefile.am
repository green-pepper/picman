## Process this file with automake to produce Makefile.in

libpicmanui = $(top_builddir)/libpicman/libpicmanui-$(PICMAN_API_VERSION).la
libpicmanconfig = $(top_builddir)/libpicmanconfig/libpicmanconfig-$(PICMAN_API_VERSION).la
libpicmanwidgets = $(top_builddir)/libpicmanwidgets/libpicmanwidgets-$(PICMAN_API_VERSION).la
libpicman = $(top_builddir)/libpicman/libpicman-$(PICMAN_API_VERSION).la
libpicmancolor = $(top_builddir)/libpicmancolor/libpicmancolor-$(PICMAN_API_VERSION).la
libpicmanbase = $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la
libpicmanmath = $(top_builddir)/libpicmanmath/libpicmanmath-$(PICMAN_API_VERSION).la

libexecdir = $(picmanplugindir)/plug-ins

libexec_PROGRAMS = twain


if OS_WIN32
twain_LDFLAGS = -mwindows -luser32

twainplatform = tw_win.c
endif

if HAVE_WINDRES
include $(top_srcdir)/build/windows/picmanrc-plug-ins.rule
twain_RC = twain.rc.o
endif

if HAVE_MAC_TWAIN
twain_LDFLAGS = -framework Carbon -framework TWAIN

twainplatform = tw_mac.c

twaindatadir = $(picmandatadir)/twain
twaindata_DATA = picman-twain.png
endif


twain_SOURCES = \
	tw_func.c	\
	tw_func.h	\
	tw_util.c	\
	tw_util.h	\
	twain.c		\
	twain.h		\
	tw_local.h	\
	tw_platform.h	\
	$(twainplatform)

EXTRA_DIST = \
	README		\
	picman-twain.png	\
	tw_dump.c	\
	tw_dump.h	\
	tw_win.c	\
	tw_mac.c

INCLUDES = \
	-I$(top_srcdir)	\
	$(GTK_CFLAGS)	\
	$(GEGL_CFLAGS) \
	-I$(includedir)

LDADD = \
	$(libpicman)	\
	$(libpicmancolor)	\
	$(libpicmanbase)	\
	$(GLIB_LIBS)	\
	$(RT_LIBS)	\
	$(INTLLIBS)	\
	$(twain_LIBS)	\
	$(twain_RC)
