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
libpicmanmath = $(top_builddir)/libpicmanmath/libpicmanmath-$(PICMAN_API_VERSION).la
libpicmanbase = $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la

if HAVE_WINDRES
include $(top_srcdir)/build/windows/picmanrc-plug-ins.rule
pagecurl_RC = pagecurl.rc.o
endif

AM_LDFLAGS = $(mwindows)

libexecdir = $(picmanplugindir)/plug-ins

libexec_PROGRAMS = pagecurl

pagecurl_SOURCES = pagecurl.c
nodist_pagecurl_SOURCES = pagecurl-icons.h

BUILT_SOURCES = pagecurl-icons.h

INCLUDES = \
	-I$(top_srcdir)	\
	$(GTK_CFLAGS)	\
	$(GEGL_CFLAGS) \
	-I$(includedir)

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
	$(pagecurl_RC)


PAGECURL_IMAGES = \
	curl0.png	\
	curl1.png	\
	curl2.png	\
	curl3.png	\
	curl4.png	\
	curl5.png	\
	curl6.png	\
	curl7.png

EXTRA_DIST = $(PAGECURL_IMAGES)

CLEANFILES = $(BUILT_SOURCES) pagecurl-icons.list

pagecurl-icons.list: $(PAGECURL_IMAGES) Makefile.am
	( rm -f $@; \
          for image in $(PAGECURL_IMAGES); do \
            echo $$image | \
              sed -e 's|.*/||' -e 's|-|_|g' -e 's|\.png$$||' >> $@; \
	    echo "  $(srcdir)/$$image" >> $@; \
	  done )

$(srcdir)/pagecurl-icons.h: pagecurl-icons.list
	$(GDK_PIXBUF_CSOURCE) --raw --build-list `cat pagecurl-icons.list` > $(@F)
