## Process this file with automake to produce Makefile.in

if OS_WIN32
mwindows = -mwindows
else
libm = -lm
endif

libpicmanbase = $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la
libpicmancolor = $(top_builddir)/libpicmancolor/libpicmancolor-$(PICMAN_API_VERSION).la
libpicmanconfig = $(top_builddir)/libpicmanconfig/libpicmanconfig-$(PICMAN_API_VERSION).la
libpicmanmath = $(top_builddir)/libpicmanmath/libpicmanmath-$(PICMAN_API_VERSION).la  $(libm)
libpicmanmodule = $(top_builddir)/libpicmanmodule/libpicmanmodule-$(PICMAN_API_VERSION).la
libpicmanwidgets = $(top_builddir)/libpicmanwidgets/libpicmanwidgets-$(PICMAN_API_VERSION).la


AM_CPPFLAGS = $(GTK_CFLAGS) -DTOP_SRCDIR=\"$(top_srcdir)\"

INCLUDES = \
	-I$(top_srcdir)	\
	$(GEGL_CFLAGS)

AM_LDFLAGS = $(mwindows)


if ENABLE_GTK_DOC
noinst_PROGRAMS = $(DOC_SHOOTER)
endif

doc_shooter_SOURCES = \
	shadow.c	\
	shadow.h	\
	shooter.c	\
	units.c		\
	units.h		\
	widgets.c	\
	widgets.h

doc_shooter_LDADD = \
	$(libpicmanwidgets)	\
	$(libpicmanconfig)	\
	$(libpicmanmodule)	\
	$(libpicmanmath)		\
	$(libpicmancolor)		\
	$(libpicmanbase)		\
	$(GTK_LIBS)		\
	$(X_LIBS) -lX11 -lXext


EXTRA_PROGRAMS = doc-shooter

EXTRA_DIST = README.shooter


clean-local:
	rm -f *.png
