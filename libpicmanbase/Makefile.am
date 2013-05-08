## Process this file with automake to produce Makefile.in

if PLATFORM_WIN32
no_undefined = -no-undefined
endif

if PLATFORM_OSX
xobjective_c = "-xobjective-c"
framework_cocoa = -framework Cocoa
endif

if OS_WIN32
ole32_lib = -lole32
picmanbase_def = picmanbase.def
libpicmanbase_export_symbols = -export-symbols picmanbase.def

install-libtool-import-lib:
	$(INSTALL) .libs/libpicmanbase-$(PICMAN_API_VERSION).dll.a $(DESTDIR)$(libdir)
	$(INSTALL) picmanbase.def $(DESTDIR)$(libdir)

uninstall-libtool-import-lib:
	-rm $(DESTDIR)$(libdir)/libpicmanbase-$(PICMAN_API_VERSION).dll.a
	-rm $(DESTDIR)$(libdir)/picmanbase.def
else
install-libtool-import-lib:
uninstall-libtool-import-lib:
endif

if MS_LIB_AVAILABLE
noinst_DATA = picmanbase-$(PICMAN_API_VERSION).lib

install-ms-lib:
	$(INSTALL) picmanbase-$(PICMAN_API_VERSION).lib $(DESTDIR)$(libdir)

uninstall-ms-lib:
	-rm $(DESTDIR)$(libdir)/picmanbase-$(PICMAN_API_VERSION).lib

picmanbase-@PICMAN_API_VERSION@.lib: picmanbase.def
	lib -name:libpicmanbase-$(PICMAN_API_VERSION)-@LT_CURRENT_MINUS_AGE@.dll -def:picmanbase.def -out:$@

else
install-ms-lib:
uninstall-ms-lib:
endif

libpicmanbaseincludedir = $(includedir)/picman-$(PICMAN_API_VERSION)/libpicmanbase

AM_CPPFLAGS = \
	-DPREFIX=\""$(prefix)"\"				\
	-DPICMANDIR=\""$(picmandir)"\"				\
	-DDATADIR=\""$(picmandatadir)"\"				\
	-DLOCALEDIR=\""$(picmanlocaledir)"\"			\
	-DPLUGINDIR=\""$(picmanplugindir)"\"			\
	-DSYSCONFDIR=\""$(picmansysconfdir)"\"			\
	-DPICMAN_PACKAGE=\""@PACKAGE@"\"				\
	-DPICMAN_DATA_VERSION=\"$(PICMAN_DATA_VERSION)\"		\
	-DPICMAN_USER_VERSION=\"$(PICMAN_USER_VERSION)\"		\
	-DPICMAN_SYSCONF_VERSION=\"$(PICMAN_SYSCONF_VERSION)\"	\
	-DPICMAN_PLUGIN_VERSION=\"$(PICMAN_PLUGIN_VERSION)\"	\
	-DG_LOG_DOMAIN=\"LibPicmanBase\"				\
	-DPICMAN_BASE_COMPILATION

AM_CCASFLAGS = \
	-I$(top_builddir)	\
	-I$(top_srcdir)

INCLUDES = \
	-I$(top_srcdir)		\
	$(GLIB_CFLAGS)		\
	$(BINRELOC_CFLAGS)	\
	-I$(includedir)		\
	$(xobjective_c)

EXTRA_DIST = \
	picmanbase.def

lib_LTLIBRARIES = libpicmanbase-@PICMAN_API_VERSION@.la

# help `make' along by giving another name for the file, which it knows
# how to build
../libpicmanbase/picmanversion.h: picmanversion.h
	@:


libpicmanbase_sources = \
	picmanbase.h		\
	picmanbaseenums.h		\
	picmanbasetypes.h		\
	picmanbasetypes.c		\
	picmanlimits.h		\
	picmanparam.h		\
	picmanversion.h		\
	\
	picmanbase-private.c	\
	picmanbase-private.h	\
	picmanchecks.c           	\
	picmanchecks.h           	\
	picmancpuaccel.c		\
	picmancpuaccel.h		\
	picmandatafiles.c		\
	picmandatafiles.h		\
	picmanenv.c		\
	picmanenv.h		\
	picmanmemsize.c		\
	picmanmemsize.h		\
	picmanparasite.c		\
	picmanparasite.h		\
	picmanparasiteio.c	\
	picmanparasiteio.h	\
	picmanprotocol.c		\
	picmanprotocol.h		\
	picmanrectangle.c		\
	picmanrectangle.h		\
	picmanreloc.c		\
	picmanreloc.h		\
	picmansignal.c            \
	picmansignal.h            \
	picmanunit.c		\
	picmanunit.h		\
	picmanutils.c            	\
	picmanutils.h            	\
	picmanvaluearray.c	\
	picmanvaluearray.h	\
	picmanwin32-io.h		\
	picmanwire.c		\
	picmanwire.h

libpicmanbase_built_sources = \
	picmanbaseenums.c

libpicmanbase_@PICMAN_API_VERSION@_la_SOURCES = \
	$(libpicmanbase_sources)	\
	$(libpicmanbase_built_sources)


libpicmanbaseinclude_HEADERS = \
	picmanbase.h		\
	picmanbaseenums.h		\
	picmanbasetypes.h		\
	picmancpuaccel.h		\
	picmanlimits.h		\
	picmanparam.h		\
	picmanversion.h		\
	\
	picmanchecks.h		\
	picmandatafiles.h		\
	picmanenv.h		\
	picmanmemsize.h		\
	picmanparasite.h		\
	picmanparasiteio.h	\
	picmanrectangle.h		\
	picmansignal.h		\
	picmanunit.h		\
	picmanutils.h		\
	picmanvaluearray.h


libpicmanbase_@PICMAN_API_VERSION@_la_LDFLAGS = \
	-version-info $(LT_VERSION_INFO)	\
	$(no_undefined) 			\
	$(libpicmanbase_export_symbols)		\
	$(framework_cocoa)

libpicmanbase_@PICMAN_API_VERSION@_la_DEPENDENCIES = $(picmanbase_def)

libpicmanbase_@PICMAN_API_VERSION@_la_LIBADD = \
	$(GLIB_LIBS)	\
	$(ole32_lib)


install-data-local: install-ms-lib install-libtool-import-lib

uninstall-local: uninstall-ms-lib uninstall-libtool-import-lib

#
# test programs, not to be built by default and never installed
#

TESTS = test-cpu-accel

test_cpu_accel_SOURCES = test-cpu-accel.c

test_cpu_accel_DEPENDENCIES = \
	$(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la

test_cpu_accel_LDADD = \
	$(GLIB_LIBS)	\
	$(test_cpu_accel_DEPENDENCIES)


EXTRA_PROGRAMS = test-cpu-accel


#
# rules to generate built sources
#

gen_sources = xgen-bec
CLEANFILES = $(EXTRA_PROGRAMS) $(gen_sources)

$(srcdir)/picmanbaseenums.c: $(srcdir)/picmanbaseenums.h $(PICMAN_MKENUMS)
	$(PICMAN_MKENUMS) \
		--fhead "#include \"config.h\"\n#include <glib-object.h>\n#undef PICMAN_DISABLE_DEPRECATED\n#include \"picmanbasetypes.h\"\n#include \"libpicman/libpicman-intl.h\"" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static const G@Type@Value values[] =\n  {" \
		--vprod "    { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "    { 0, NULL, NULL }\n  };\n" \
		--dhead "  static const Picman@Type@Desc descs[] =\n  {" \
		--dprod "    { @VALUENAME@, @valuedesc@, @valuehelp@ }," \
		--dtail "    { 0, NULL, NULL }\n  };\n\n  static GType type = 0;\n\n  if (G_UNLIKELY (! type))\n    {\n      type = g_@type@_register_static (\"@EnumName@\", values);\n      picman_type_set_translation_domain (type, GETTEXT_PACKAGE \"-libpicman\");\n      picman_type_set_translation_context (type, \"@enumnick@\");\n      picman_@type@_set_value_descriptions (type, descs);\n    }\n\n  return type;\n}\n" \
		$(srcdir)/picmanbaseenums.h > xgen-bec \
	&& cp xgen-bec $(@F) \
	&& rm -f xgen-bec


DISTCLEANFILES = picmanversion.h
