## Process this file with automake to produce Makefile.in

libpicmanbase = $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la

if PLATFORM_WIN32
no_undefined = -no-undefined
endif

if PLATFORM_WIN32
else
libm = -lm
endif

if OS_WIN32
picmancolor_def = picmancolor.def
libpicmancolor_export_symbols = -export-symbols picmancolor.def

install-libtool-import-lib:
	$(INSTALL) .libs/libpicmancolor-$(PICMAN_API_VERSION).dll.a $(DESTDIR)$(libdir)
	$(INSTALL) picmancolor.def $(DESTDIR)$(libdir)

uninstall-libtool-import-lib:
	-rm $(DESTDIR)$(libdir)/libpicmancolor-$(PICMAN_API_VERSION).dll.a
	-rm $(DESTDIR)$(libdir)/picmancolor.def
else
install-libtool-import-lib:
uninstall-libtool-import-lib:
endif

if MS_LIB_AVAILABLE
noinst_DATA = picmancolor-$(PICMAN_API_VERSION).lib

install-ms-lib:
	$(INSTALL) picmancolor-$(PICMAN_API_VERSION).lib $(DESTDIR)$(libdir)

uninstall-ms-lib:
	-rm $(DESTDIR)$(libdir)/picmancolor-$(PICMAN_API_VERSION).lib

picmancolor-@PICMAN_API_VERSION@.lib: picmancolor.def
	lib -name:libpicmancolor-$(PICMAN_API_VERSION)-@LT_CURRENT_MINUS_AGE@.dll -def:picmancolor.def -out:$@

else
install-ms-lib:
uninstall-ms-lib:
endif

libpicmancolorincludedir = $(includedir)/picman-$(PICMAN_API_VERSION)/libpicmancolor

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"LibPicmanColor\"	\
	-DPICMAN_COLOR_COMPILATION

INCLUDES = \
	-I$(top_srcdir)		\
	$(GEGL_CFLAGS)		\
	$(CAIRO_CFLAGS)		\
	$(GDK_PIXBUF_CFLAGS)	\
	-I$(includedir)

EXTRA_DIST = \
	picmancolor.def

lib_LTLIBRARIES = libpicmancolor-@PICMAN_API_VERSION@.la

libpicmancolor_@PICMAN_API_VERSION@_la_SOURCES = \
	picmancolor.h			\
	picmancolortypes.h		\
	picmanadaptivesupersample.c	\
	picmanadaptivesupersample.h	\
	picmanbilinear.c			\
	picmanbilinear.h			\
	picmancairo.c			\
	picmancairo.h			\
	picmancmyk.c			\
	picmancmyk.h			\
	picmancolormanaged.c		\
	picmancolormanaged.h		\
	picmancolorspace.c		\
	picmancolorspace.h		\
	picmanhsl.c			\
	picmanhsl.h			\
	picmanhsv.c			\
	picmanhsv.h			\
	picmanpixbuf.c			\
	picmanpixbuf.h			\
	picmanrgb.c			\
	picmanrgb.h			\
	picmanrgb-parse.c

libpicmancolorinclude_HEADERS = \
	picmancolor.h			\
	picmancolortypes.h		\
	picmanadaptivesupersample.h	\
	picmanbilinear.h			\
	picmancairo.h			\
	picmancmyk.h			\
	picmancolormanaged.h		\
	picmancolorspace.h		\
	picmanhsl.h			\
	picmanhsv.h			\
	picmanpixbuf.h			\
	picmanrgb.h

libpicmancolor_@PICMAN_API_VERSION@_la_LDFLAGS = \
	-version-info $(LT_VERSION_INFO)	\
	$(no_undefined) 			\
	$(libpicmancolor_export_symbols)

libpicmancolor_@PICMAN_API_VERSION@_la_DEPENDENCIES = \
	$(picmancolor_def)	\
	$(libpicmanbase)

libpicmancolor_@PICMAN_API_VERSION@_la_LIBADD = \
	$(GEGL_LIBS)		\
	$(CAIRO_LIBS)		\
	$(GDK_PIXBUF_LIBS)	\
	$(libm)


#
# test programs, not to be built by default and never installed
#

TESTS = test-color-parser$(EXEEXT)

EXTRA_PROGRAMS = test-color-parser

test_color_parser_DEPENDENCIES = \
	$(top_builddir)/libpicmancolor/libpicmancolor-$(PICMAN_API_VERSION).la

test_color_parser_LDADD = \
	$(CAIRO_LIBS) 		\
	$(GLIB_LIBS) 		\
	$(test_color_parser_DEPENDENCIES)


CLEANFILES = $(EXTRA_PROGRAMS)


install-data-local: install-ms-lib install-libtool-import-lib

uninstall-local: uninstall-ms-lib uninstall-libtool-import-lib
