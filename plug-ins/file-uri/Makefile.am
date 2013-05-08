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
file_uri_RC = file-uri.rc.o
endif

AM_LDFLAGS = $(mwindows)

libexecdir = $(picmanplugindir)/plug-ins

libexec_PROGRAMS = file-uri

if HAVE_GVFS
backend_sources = uri-backend-gvfs.c
backend_cflags = $(GIO_CFLAGS)
backend_libs = $(GIO_LIBS)
else
if HAVE_LIBCURL
backend_sources = uri-backend-libcurl.c
backend_cflags = $(CURL_CFLAGS)
backend_libs = $(CURL_LIBS)
else
backend_sources = uri-backend-wget.c
backend_cflags = 
backend_libs = 
endif
endif

file_uri_SOURCES = \
	uri.c			\
	uri-backend.h		\
	$(backend_sources)

INCLUDES = \
	-I$(top_srcdir)		\
	$(backend_cflags)	\
	$(GTK_CFLAGS)		\
	$(GEGL_CFLAGS) \
	-I$(includedir)

LDADD = \
	$(libpicmanui)		\
	$(libpicmanwidgets)	\
	$(libpicmanconfig)	\
	$(libpicmanmath)		\
	$(libpicman)		\
	$(libpicmancolor)		\
	$(libpicmanbase)		\
	$(backend_libs)		\
	$(GTK_LIBS)		\
	$(RT_LIBS)		\
	$(INTLLIBS)		\
	$(file_uri_RC)
