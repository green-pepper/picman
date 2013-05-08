## Process this file with automake to produce Makefile.in

if PLATFORM_WIN32
no_undefined = -no-undefined
endif

if PLATFORM_OSX
xobjective_c = "-xobjective-c"
framework_cocoa = -framework Cocoa
endif

if OS_WIN32
picmanthumb_def = picmanthumb.def
libpicmanthumb_export_symbols = -export-symbols picmanthumb.def

install-libtool-import-lib:
	$(INSTALL) .libs/libpicmanthumb-$(PICMAN_API_VERSION).dll.a $(DESTDIR)$(libdir)
	$(INSTALL) picmanthumb.def $(DESTDIR)$(libdir)

uninstall-libtool-import-lib:
	-rm $(DESTDIR)$(libdir)/libpicmanthumb-$(PICMAN_API_VERSION).dll.a
	-rm $(DESTDIR)$(libdir)/picmanthumb.def
else
install-libtool-import-lib:
uninstall-libtool-import-lib:
endif

if MS_LIB_AVAILABLE
noinst_DATA = picmanthumb-$(PICMAN_API_VERSION).lib

install-ms-lib:
	$(INSTALL) picmanthumb-$(PICMAN_API_VERSION).lib $(DESTDIR)$(libdir)

uninstall-ms-lib:
	-rm $(DESTDIR)$(libdir)/picmanthumb-$(PICMAN_API_VERSION).lib

picmanthumb-@PICMAN_API_VERSION@.lib: picmanthumb.def
	lib -name:libpicmanthumb-$(PICMAN_API_VERSION)-@LT_CURRENT_MINUS_AGE@.dll -def:picmanthumb.def -out:$@

else
install-ms-lib:
uninstall-ms-lib:
endif

libpicmanthumbincludedir = $(includedir)/picman-$(PICMAN_API_VERSION)/libpicmanthumb

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"LibPicmanThumb\"	\
	-DPICMAN_THUMB_COMPILATION

INCLUDES = \
	-I$(top_srcdir)		\
	$(GDK_PIXBUF_CFLAGS)	\
	$(GLIB_CFLAGS)		\
	-I$(includedir)		\
	$(xobjective_c)

EXTRA_DIST = \
	picmanthumb.def

lib_LTLIBRARIES = libpicmanthumb-@PICMAN_API_VERSION@.la

libpicmanthumb_@PICMAN_API_VERSION@_la_SOURCES = \
	picmanthumb.h		\
	picmanthumb-enums.c	\
	picmanthumb-enums.h	\
	picmanthumb-error.c	\
	picmanthumb-error.h	\
	picmanthumb-types.h	\
	picmanthumb-utils.c	\
	picmanthumb-utils.h	\
	picmanthumbnail.c		\
	picmanthumbnail.h

libpicmanthumbinclude_HEADERS = \
	picmanthumb.h		\
	picmanthumb-enums.h	\
	picmanthumb-error.h	\
	picmanthumb-types.h	\
	picmanthumb-utils.h	\
	picmanthumbnail.h

libpicmanthumb_@PICMAN_API_VERSION@_la_LDFLAGS = \
	-version-info $(LT_VERSION_INFO)	\
	$(no_undefined)				\
	$(libpicmanthumb_export_symbols)		\
	$(framework_cocoa)

libpicmanthumb_@PICMAN_API_VERSION@_la_LIBADD = \
	$(GDK_PIXBUF_LIBS)	\
	$(GLIB_LIBS) 


noinst_PROGRAMS = picman-thumbnail-list

picman_thumbnail_list_SOURCES = picman-thumbnail-list.c

picman_thumbnail_list_LDADD = \
	libpicmanthumb-$(PICMAN_API_VERSION).la \
	$(GDK_PIXBUF_LIBS) \
	$(GLIB_LIBS)


install-data-local: install-ms-lib install-libtool-import-lib

uninstall-local: uninstall-ms-lib uninstall-libtool-import-lib


#
# rules to generate built sources
#
# setup autogeneration dependencies
gen_sources = xgen-tec
CLEANFILES = $(gen_sources)

picmanthumb-enums.c: $(srcdir)/picmanthumb-enums.h $(PICMAN_MKENUMS)
	$(PICMAN_MKENUMS) \
		--fhead "#include \"config.h\"\n#include <glib-object.h>\n#include \"picmanthumb-enums.h\"" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static const G@Type@Value values[] =\n  {" \
		--vprod "    { @VALUENAME@, @valuedesc@, \"@valuenick@\" }," \
		--vtail "    { 0, NULL, NULL }\n  };\n\n  static GType type = 0;\n\n  if (G_UNLIKELY (! type))\n    type = g_@type@_register_static (\"@EnumName@\", values);\n\n  return type;\n}\n" \
		$(srcdir)/picmanthumb-enums.h > xgen-tec \
	&& cp xgen-tec $(@F) \
	&& rm -f xgen-tec
