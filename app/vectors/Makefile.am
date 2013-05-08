## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Picman-Vectors\"

INCLUDES = \
	-I$(top_builddir)	\
	-I$(top_srcdir)		\
	-I$(top_builddir)/app	\
	-I$(top_srcdir)/app	\
	$(CAIRO_CFLAGS)		\
	$(GEGL_CFLAGS)		\
	$(GDK_PIXBUF_CFLAGS)	\
	-I$(includedir)

noinst_LIBRARIES = libappvectors.a

libappvectors_a_SOURCES = \
	vectors-enums.h		\
	vectors-types.h		\
	picmananchor.c		\
	picmananchor.h		\
	picmanbezierstroke.h	\
	picmanbezierstroke.c	\
	picmanstroke.h		\
	picmanstroke.c		\
	picmanstroke-new.h	\
	picmanstroke-new.c	\
	picmanvectors.c		\
	picmanvectors.h		\
	picmanvectors-compat.c	\
	picmanvectors-compat.h	\
	picmanvectors-export.c	\
	picmanvectors-export.h	\
	picmanvectors-import.c	\
	picmanvectors-import.h	\
	picmanvectors-preview.c	\
	picmanvectors-preview.h	\
	picmanvectors-warp.c	\
	picmanvectors-warp.h	\
	picmanvectorsmodundo.c	\
	picmanvectorsmodundo.h	\
	picmanvectorspropundo.c	\
	picmanvectorspropundo.h	\
	picmanvectorsundo.c	\
	picmanvectorsundo.h
