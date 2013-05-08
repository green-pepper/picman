## Process this file with automake to produce Makefile.in

libpicmanbase = $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la

if WITH_PDBGEN
D_pdbgen = pdbgen
else
D_pdbgen =
endif

SUBDIRS = $(D_pdbgen)

if OS_WIN32

bin_PROGRAMS = \
	picmantool-2.0 \
	picman-debug-resume

picman_debug_resume_SOURCES = picman-debug-resume.c

else

bin_PROGRAMS = picmantool-2.0

endif

noinst_PROGRAMS = test-clipboard

EXTRA_PROGRAMS = \
	kernelgen

picmantool_2_0_SOURCES = \
	picmantool.c

picmantool_2_0_LDADD = \
	$(libpicmanbase)			\
	$(GTK_LIBS)

kernelgen_SOURCES = kernelgen.c

test_clipboard_SOURCES = test-clipboard.c

test_clipboard_LDADD = \
	$(GTK_LIBS)

AM_CPPFLAGS = \
	-DPICMAN_APP_VERSION=\"@PICMAN_APP_VERSION@\"	\
	-DLOCALEDIR=\""$(picmanlocaledir)"\"		\
	-DPREFIX=\""$(prefix)"\"			\
	-DEXEC_PREFIX=\""$(exec_prefix)"\"		\
	-DBINDIR=\""$(bindir)"\"			\
	-DSBINDIR=\""$(sbindir)"\"			\
	-DLIBEXECDIR=\""$(libexecdir)"\"		\
	-DDATADIR=\""$(datadir)"\"			\
	-DDATAROOTDIR=\""$(datarootdir)"\"		\
	-DSYSCONFDIR=\""$(sysconfdir)"\"		\
	-DSHAREDSTATEDIR=\""$(sharedstatedir)"\"	\
	-DLOCALSTATEDIR=\""$(localstatedir)"\"		\
	-DLIBDIR=\""$(libdir)"\"			\
	-DINFODIR=\""$(infodir)"\"			\
	-DMANDIR=\""$(mandir)"\"			\
	-DPICMANPLUGINDIR=\""$(picmanplugindir)"\"		\
	-DPICMANDATADIR=\""$(picmandatadir)"\"		\
	-DCC=\""$(CC)"\"				\
	-DPICMANDIR=\""$(picmandir)"\"			\
	-DPICMAN_PLUGIN_VERSION=\""$(PICMAN_PLUGIN_VERSION)"\"

INCLUDES = \
	-I$(top_srcdir)	\
	$(GTK_CFLAGS)	\
	-I$(includedir)

EXTRA_DIST = \
	defcheck.py	\
	picman-mkenums	\
	picmanpath2svg.py
