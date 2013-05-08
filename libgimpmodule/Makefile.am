## Process this file with automake to produce Makefile.in

libpicmanbase = $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la

if PLATFORM_WIN32
no_undefined = -no-undefined
endif

if OS_WIN32
picmanmodule_def = picmanmodule.def
libpicmanmodule_export_symbols = -export-symbols picmanmodule.def

install-libtool-import-lib:
	$(INSTALL) .libs/libpicmanmodule-$(PICMAN_API_VERSION).dll.a $(DESTDIR)$(libdir)
	$(INSTALL) picmanmodule.def $(DESTDIR)$(libdir)

uninstall-libtool-import-lib:
	-rm $(DESTDIR)$(libdir)/libpicmanmodule-$(PICMAN_API_VERSION).dll.a
	-rm $(DESTDIR)$(libdir)/picmanmodule.def
else
install-libtool-import-lib:
uninstall-libtool-import-lib:
endif

if MS_LIB_AVAILABLE
noinst_DATA = picmanmodule-$(PICMAN_API_VERSION).lib

install-ms-lib:
	$(INSTALL) picmanmodule-$(PICMAN_API_VERSION).lib $(DESTDIR)$(libdir)

uninstall-ms-lib:
	-rm $(DESTDIR)$(libdir)/picmanmodule-$(PICMAN_API_VERSION).lib

picmanmodule-@PICMAN_API_VERSION@.lib: picmanmodule.def
	lib -name:libpicmanmodule-$(PICMAN_API_VERSION)-@LT_CURRENT_MINUS_AGE@.dll -def:picmanmodule.def -out:$@

else
install-ms-lib:
uninstall-ms-lib:
endif

libpicmanmoduleincludedir = $(includedir)/picman-$(PICMAN_API_VERSION)/libpicmanmodule

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"LibPicmanModule\"	\
	-DPICMAN_MODULE_COMPILATION

INCLUDES = \
	-I$(top_srcdir)			\
	$(GLIB_CFLAGS)			\
	$(GMODULE_NO_EXPORT_CFLAGS)	\
	-I$(includedir)

EXTRA_DIST = \
	picmanmodule.def

lib_LTLIBRARIES = libpicmanmodule-@PICMAN_API_VERSION@.la

libpicmanmodule_@PICMAN_API_VERSION@_la_SOURCES = \
	picmanmoduletypes.h	\
	picmanmodule.c		\
	picmanmodule.h		\
	picmanmoduledb.c		\
	picmanmoduledb.h

libpicmanmoduleinclude_HEADERS = \
	picmanmoduletypes.h	\
	picmanmodule.h		\
	picmanmoduledb.h

libpicmanmodule_@PICMAN_API_VERSION@_la_LDFLAGS = \
	-version-info $(LT_VERSION_INFO)	\
	$(no_undefined) 			\
	$(libpicmanmodule_export_symbols)

libpicmanmodule_@PICMAN_API_VERSION@_la_DEPENDENCIES = $(picmanmodule_def)

libpicmanmodule_@PICMAN_API_VERSION@_la_LIBADD = \
	$(libpicmanbase)			\
	$(GMODULE_NO_EXPORT_LIBS) 	\
	$(GLIB_LIBS)

install-data-local: install-ms-lib install-libtool-import-lib

uninstall-local: uninstall-ms-lib uninstall-libtool-import-lib
