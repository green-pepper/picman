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

SUBDIRS = Brushes Paper Presets

if HAVE_WINDRES
include $(top_srcdir)/build/windows/picmanrc-plug-ins.rule
picmanressionist_RC = picmanressionist.rc.o
endif

AM_CPPFLAGS = \
	-DDEFAULTPATH=\""~/$(picmandir)/picmanressionist:$(picmandatadir)/picmanressionist"\"

AM_LDFLAGS = $(mwindows)

INCLUDES = \
	-I$(top_srcdir)	\
	$(GTK_CFLAGS)	\
	$(GEGL_CFLAGS) \
	-I$(includedir)

libexecdir = $(picmanplugindir)/plug-ins

libexec_PROGRAMS = picmanressionist

picmanressionist_SOURCES = \
	brush.c			\
	brush.h			\
	color.c			\
	color.h			\
	general.c		\
	general.h		\
	picman.c			\
	picmanressionist.c	\
	picmanressionist.h	\
	globals.c		\
	infile.h        	\
	orientation.h		\
	orientation.c		\
	orientmap.h     \
	orientmap.c		\
	paper.c			\
	paper.h			\
	placement.c		\
	placement.h		\
	plasma.c		\
	ppmtool.c		\
	ppmtool.h		\
	presets.c		\
	presets.h		\
	preview.c		\
	preview.h		\
	random.h        	\
	repaint.c		\
	size.h          	\
	size.c			\
	sizemap.c		\
	utils.c

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
	$(picmanressionist_RC)

EXTRA_DIST = README
