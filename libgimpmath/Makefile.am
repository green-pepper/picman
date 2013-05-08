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
picmanmath_def = picmanmath.def
libpicmanmath_export_symbols = -export-symbols picmanmath.def

install-libtool-import-lib:
	$(INSTALL) .libs/libpicmanmath-$(PICMAN_API_VERSION).dll.a $(DESTDIR)$(libdir)
	$(INSTALL) picmanmath.def $(DESTDIR)$(libdir)

uninstall-libtool-import-lib:
	-rm $(DESTDIR)$(libdir)/libpicmanmath-$(PICMAN_API_VERSION).dll.a
	-rm $(DESTDIR)$(libdir)/picmanmath.def
else
install-libtool-import-lib:
uninstall-libtool-import-lib:
endif

if MS_LIB_AVAILABLE
noinst_DATA = picmanmath-$(PICMAN_API_VERSION).lib

install-ms-lib:
	$(INSTALL) picmanmath-$(PICMAN_API_VERSION).lib $(DESTDIR)$(libdir)

uninstall-ms-lib:
	-rm $(DESTDIR)$(libdir)/picmanmath-$(PICMAN_API_VERSION).lib

picmanmath-@PICMAN_API_VERSION@.lib: picmanmath.def
	lib -name:libpicmanmath-$(PICMAN_API_VERSION)-@LT_CURRENT_MINUS_AGE@.dll -def:picmanmath.def -out:$@

else
install-ms-lib:
uninstall-ms-lib:
endif

libpicmanmathincludedir = $(includedir)/picman-$(PICMAN_API_VERSION)/libpicmanmath

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"LibPicmanMath\"	\
	-DPICMAN_MATH_COMPILATION

INCLUDES = \
	-I$(top_srcdir)	\
	$(GLIB_CFLAGS)	\
	-I$(includedir)

EXTRA_DIST = \
	picmanmath.def

lib_LTLIBRARIES = libpicmanmath-@PICMAN_API_VERSION@.la

libpicmanmath_@PICMAN_API_VERSION@_la_SOURCES = \
	picmanmath.h	\
	picmanmathtypes.h	\
	picmanmatrix.c	\
	picmanmatrix.h	\
	picmanmd5.c	\
	picmanmd5.h	\
	picmanvector.c	\
	picmanvector.h

libpicmanmathinclude_HEADERS = \
	picmanmath.h	\
	picmanmathtypes.h	\
	picmanmatrix.h	\
	picmanmd5.h	\
	picmanvector.h

libpicmanmath_@PICMAN_API_VERSION@_la_LDFLAGS = \
	-version-info $(LT_VERSION_INFO)	\
	$(no_undefined)				\
	$(libpicmanmath_export_symbols)

libpicmanmath_@PICMAN_API_VERSION@_la_DEPENDENCIES = \
	$(picmanmath_def)	\
	$(libpicmanbase)

libpicmanmath_@PICMAN_API_VERSION@_la_LIBADD = \
	$(GLIB_LIBS)	\
	$(libm)


install-data-local: install-ms-lib install-libtool-import-lib

uninstall-local: uninstall-ms-lib uninstall-libtool-import-lib
