## Process this file with automake to produce Makefile.in

libpicmanui = $(top_builddir)/libpicman/libpicmanui-$(PICMAN_API_VERSION).la
libpicmanconfig = $(top_builddir)/libpicmanconfig/libpicmanconfig-$(PICMAN_API_VERSION).la
libpicmanwidgets = $(top_builddir)/libpicmanwidgets/libpicmanwidgets-$(PICMAN_API_VERSION).la
libpicman = $(top_builddir)/libpicman/libpicman-$(PICMAN_API_VERSION).la
libpicmancolor = $(top_builddir)/libpicmancolor/libpicmancolor-$(PICMAN_API_VERSION).la
libpicmanbase = $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la
libpicmanmath = $(top_builddir)/libpicmanmath/libpicmanmath-$(PICMAN_API_VERSION).la
libpicmanthumb = $(top_builddir)/libpicmanthumb/libpicmanthumb-$(PICMAN_API_VERSION).la

SUBDIRS = plug-ins

if PLATFORM_WIN32
no_undefined = -no-undefined
shrext       = -shrext .pyd
path_separator = ;
else
path_separator = :
endif

AM_CFLAGS = $(PYPICMAN_EXTRA_CFLAGS)

INCLUDES = -I$(top_srcdir) $(PYTHON_INCLUDES) $(PYGTK_CFLAGS) $(GTK_CFLAGS) $(GEGL_CFLAGS)

pypicmanbase = python

pypicmandir = $(picmanplugindir)/$(pypicmanbase)

pypicman_LTLIBRARIES = picman.la _picmanenums.la picmancolor.la _picmanui.la \
	picmanthumb.la

picman_la_SOURCES = \
	picmanmodule.c		\
	pypicman-item.c		\
	pypicman-display.c	\
	pypicman-drawable.c	\
	pypicman-image.c		\
	pypicman-parasite.c	\
	pypicman-pdb.c		\
	pypicman-tile.c		\
	pypicman-vectors.c	\
	pypicman.h		\
	pypicman-api.h		\
	pypicman-intl.h		\
	pypicman-util.h

picman_la_LDFLAGS = -module -avoid-version $(no_undefined) $(shrext) \
	-export-symbols-regex initpicman

picman_la_LIBADD = \
	$(libpicman)		\
	$(libpicmancolor)		\
	$(libpicmanbase)		\
	$(GLIB_LIBS)		\
	$(PYLINK_LIBS)		\
	$(RT_LIBS)

_picmanenums_la_SOURCES = \
	picmanenumsmodule.c

_picmanenums_la_LDFLAGS = -module -avoid-version $(no_undefined) $(shrext) \
	-export-symbols-regex init_picmanenums

_picmanenums_la_LIBADD = \
	$(libpicman)		\
	$(libpicmancolor)		\
	$(libpicmanbase)		\
	$(GLIB_LIBS)		\
	$(PYLINK_LIBS)		\
	$(RT_LIBS)

picmancolor_la_SOURCES = \
	picmancolormodule.c	\
	pypicman-colors.c		\
	pypicmancolor.h		\
	pypicmancolor-api.h

picmancolor_la_LDFLAGS = -module -avoid-version $(no_undefined) $(shrext) \
	-export-symbols-regex initpicmancolor

picmancolor_la_LIBADD = \
	$(libpicmancolor)		\
	$(GLIB_LIBS)		\
	$(PYLINK_LIBS)

_picmanui_la_SOURCES = \
	picmanuimodule.c		\
	picmanui.c

_picmanui_la_LDFLAGS = -module -avoid-version $(no_undefined) $(shrext) \
	-export-symbols-regex init_picmanui

_picmanui_la_LIBADD = \
	$(libpicmanui)		\
	$(libpicmanwidgets)	\
	$(libpicmanconfig)	\
	$(libpicmanmath)		\
	$(libpicman)		\
	$(libpicmancolor)		\
	$(libpicmanbase)		\
	$(GTK_LIBS)		\
	$(PYLINK_LIBS)		\
	$(RT_LIBS)		\
	$(INTLLIBS)

picmanthumb_la_SOURCES = \
	picmanthumbmodule.c \
	picmanthumb.c

picmanthumb_la_LDFLAGS = -module -avoid-version $(no_undefined) $(shrext) \
	-export-symbols-regex initpicmanthumb

picmanthumb_la_LIBADD = \
	$(libpicmanthumb)		\
	$(GDK_PIXBUF_LIBS)	\
	$(PYLINK_LIBS)

pypicman_PYTHON = \
	picmanenums.py	\
	picmanfu.py	\
	picmanplugin.py	\
	picmanshelf.py	\
	picmanui.py

pypicman_DATA = pypicman-logo.png

codegen_files = \
	picmancolor-types.defs	\
	picmanenums-types.defs	\
	picmanthumb.defs		\
	picmanthumb.override	\
	picmanui.defs		\
	picmanui.override

pyenvdir = $(picmanplugindir)/environ
pyinterpdir = $(picmanplugindir)/interpreters

pyenvfile = $(pyenvdir)/pypicman.env
pyinterpfile = $(pyinterpdir)/pypicman.interp

install-env-file:
	$(mkinstalldirs) '$(DESTDIR)$(pyenvdir)'
	echo '$(path_separator) PYTHONPATH=$${picman_plug_in_dir}/$(pypicmanbase)' > '$(DESTDIR)$(pyenvfile)'

install-interp-file:
	$(mkinstalldirs) '$(DESTDIR)$(pyinterpdir)'
	echo 'python=$(PYBIN_PATH)' > '$(DESTDIR)$(pyinterpfile)'
	echo '/usr/bin/python=$(PYBIN_PATH)' >> '$(DESTDIR)$(pyinterpfile)'
	echo ':Python:E::py::python:' >> '$(DESTDIR)$(pyinterpfile)'

install-data-local: install-env-file install-interp-file

uninstall-local:
	rm -f '$(DESTDIR)$(pyenvfile)' '$(DESTDIR)$(pyinterpfile)'

EXTRA_DIST = \
	$(pypicman_DATA)		\
	$(codegen_files)

picmanui.c: picmanui.defs picmanui.override picmancolor-types.defs picmanenums-types.defs

picmanthumb.c: picmanthumb.defs picmanthumb.override

CLEANFILES = picmanui.c picmanthumb.c

.defs.c:
	(cd $(srcdir) \
	 && $(PYGTK_CODEGEN) \
	    --override $*.override \
	    --register $(PYGTK_DEFSDIR)/gdk-types.defs \
	    --register $(PYGTK_DEFSDIR)/gtk-types.defs \
	    --register picmancolor-types.defs \
	    --register picmanenums-types.defs \
	    --prefix $* $*.defs) > gen-$*.c \
	 && cp gen-$*.c $*.c \
	 && rm -f gen-$*.c
