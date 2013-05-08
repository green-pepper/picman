## Process this file with automake to produce Makefile.in

libpicmanbase = $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la
libpicmancolor = $(top_builddir)/libpicmancolor/libpicmancolor-$(PICMAN_API_VERSION).la
libpicmanmath = $(top_builddir)/libpicmanmath/libpicmanmath-$(PICMAN_API_VERSION).la

if PLATFORM_WIN32
no_undefined = -no-undefined
endif

if OS_WIN32
picmanconfig_def = picmanconfig.def
libpicmanconfig_export_symbols = -export-symbols picmanconfig.def

install-libtool-import-lib:
	$(INSTALL) .libs/libpicmanconfig-$(PICMAN_API_VERSION).dll.a $(DESTDIR)$(libdir)
	$(INSTALL) picmanconfig.def $(DESTDIR)$(libdir)

uninstall-libtool-import-lib:
	-rm $(DESTDIR)$(libdir)/libpicmanconfig-$(PICMAN_API_VERSION).dll.a
	-rm $(DESTDIR)$(libdir)/picmanconfig.def
else
install-libtool-import-lib:
uninstall-libtool-import-lib:
endif

if MS_LIB_AVAILABLE
noinst_DATA = picmanconfig-$(PICMAN_API_VERSION).lib

install-ms-lib:
	$(INSTALL) picmanconfig-$(PICMAN_API_VERSION).lib $(DESTDIR)$(libdir)

uninstall-ms-lib:
	-rm $(DESTDIR)$(libdir)/picmanconfig-$(PICMAN_API_VERSION).lib

picmanconfig-@PICMAN_API_VERSION@.lib: picmanconfig.def
	lib -name:libpicmanconfig-$(PICMAN_API_VERSION)-@LT_CURRENT_MINUS_AGE@.dll -def:picmanconfig.def -out:$@

else
install-ms-lib:
uninstall-ms-lib:
endif

libpicmanconfigincludedir = $(includedir)/picman-$(PICMAN_API_VERSION)/libpicmanconfig

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"LibPicmanConfig\"	\
	-DPICMAN_CONFIG_COMPILATION

INCLUDES = \
	-I$(top_srcdir)		\
	$(GEGL_CFLAGS)		\
	$(CAIRO_CFLAGS)		\
	$(GDK_PIXBUF_CFLAGS)	\
	-I$(includedir)

EXTRA_DIST = \
	picmanconfig.def

lib_LTLIBRARIES = libpicmanconfig-@PICMAN_API_VERSION@.la

libpicmanconfig_sources = 		\
	picmanconfig.h			\
	picmanconfigtypes.h		\
	picmanconfig-iface.c		\
	picmanconfig-iface.h		\
	picmanconfig-deserialize.c	\
	picmanconfig-deserialize.h	\
	picmanconfig-error.c		\
	picmanconfig-error.h		\
	picmanconfig-params.h		\
	picmanconfig-path.c		\
	picmanconfig-path.h		\
	picmanconfig-serialize.c		\
	picmanconfig-serialize.h		\
	picmanconfig-utils.c		\
	picmanconfig-utils.h		\
	picmanconfigwriter.c		\
	picmanconfigwriter.h		\
	picmanscanner.c			\
	picmanscanner.h			\
	picmancolorconfig.c		\
	picmancolorconfig.h		\
	picmancolorconfig-enums.h

libpicmanconfig_built_sources = \
	picmancolorconfig-enums.c

libpicmanconfig_@PICMAN_API_VERSION@_la_SOURCES = \
	$(libpicmanconfig_sources)	\
	$(libpicmanconfig_built_sources)


libpicmanconfiginclude_HEADERS = \
	picmanconfig.h			\
	picmanconfigtypes.h		\
	picmanconfig-iface.h		\
	picmanconfig-deserialize.h	\
	picmanconfig-error.h		\
	picmanconfig-params.h		\
	picmanconfig-path.h		\
	picmanconfig-serialize.h		\
	picmanconfig-utils.h		\
	picmanconfigwriter.h		\
	picmanscanner.h			\
	picmancolorconfig.h		\
	picmancolorconfig-enums.h


libpicmanconfig_@PICMAN_API_VERSION@_la_LDFLAGS = \
	-version-info $(LT_VERSION_INFO)	\
	$(no_undefined) 			\
	$(libpicmanconfig_export_symbols)

libpicmanconfig_@PICMAN_API_VERSION@_la_DEPENDENCIES = $(picmanconfig_def)

libpicmanconfig_@PICMAN_API_VERSION@_la_LIBADD = \
	$(libpicmanbase)	\
	$(libpicmancolor)	\
	$(libpicmanmath)	\
	$(GEGL_LIBS)	\
	$(CAIRO_LIBS)	\
	$(GDK_PIXBUF_LIBS)


install-data-local: install-ms-lib install-libtool-import-lib

uninstall-local: uninstall-ms-lib uninstall-libtool-import-lib


#
# rules to generate built sources
#

gen_sources = xgen-cec
CLEANFILES = $(gen_sources)

$(srcdir)/picmancolorconfig-enums.c: $(srcdir)/picmancolorconfig-enums.h $(PICMAN_MKENUMS)
	$(PICMAN_MKENUMS) \
		--fhead "#include \"config.h\"\n#include <glib-object.h>\n#include \"libpicmanbase/picmanbase.h\"\n#include \"picmancolorconfig-enums.h\"\n#include \"libpicman/libpicman-intl.h\"" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static const G@Type@Value values[] =\n  {" \
		--vprod "    { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "    { 0, NULL, NULL }\n  };\n" \
		--dhead "  static const Picman@Type@Desc descs[] =\n  {" \
		--dprod "    { @VALUENAME@, @valuedesc@, @valuehelp@ }," \
		--dtail "    { 0, NULL, NULL }\n  };\n\n  static GType type = 0;\n\n  if (G_UNLIKELY (! type))\n    {\n      type = g_@type@_register_static (\"@EnumName@\", values);\n      picman_type_set_translation_domain (type, GETTEXT_PACKAGE \"-libpicman\");\n      picman_type_set_translation_context (type, \"@enumnick@\");\n      picman_@type@_set_value_descriptions (type, descs);\n    }\n\n  return type;\n}\n" \
		$(srcdir)/picmancolorconfig-enums.h > xgen-cec \
	&& cp xgen-cec $(@F) \
	&& rm -f xgen-cec
