## Process this file with automake to produce Makefile.in

libpicmanbase = $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la
libpicmanconfig = $(top_builddir)/libpicmanconfig/libpicmanconfig-$(PICMAN_API_VERSION).la
libpicmancolor = $(top_builddir)/libpicmancolor/libpicmancolor-$(PICMAN_API_VERSION).la
libpicmanmath = $(top_builddir)/libpicmanmath/libpicmanmath-$(PICMAN_API_VERSION).la
libpicmanmodule = $(top_builddir)/libpicmanmodule/libpicmanmodule-$(PICMAN_API_VERSION).la
libpicmanthumb = $(top_builddir)/libpicmanthumb/libpicmanthumb-$(PICMAN_API_VERSION).la

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Picman-Config\"	\
	-DPICMAN_APP_VERSION_STRING=\"$(PICMAN_APP_VERSION)\"

INCLUDES = \
	-I$(top_builddir)	\
	-I$(top_srcdir)		\
	-I$(top_builddir)/app	\
	-I$(top_srcdir)/app	\
	$(GEGL_CFLAGS)		\
	$(CAIRO_CFLAGS)		\
	$(GDK_PIXBUF_CFLAGS)	\
	-I$(includedir)

noinst_LIBRARIES = libappconfig.a

libappconfig_a_sources = \
	config-enums.h			\
	config-types.h			\
	picmanconfig-dump.c		\
	picmanconfig-dump.h		\
	picmanconfig-file.c		\
	picmanconfig-file.h		\
	picmanconfig-utils.c		\
	picmanconfig-utils.h		\
	picmancoreconfig.c		\
	picmancoreconfig.h		\
	picmandisplayconfig.c		\
	picmandisplayconfig.h		\
	picmandisplayoptions.c		\
	picmandisplayoptions.h		\
	picmangeglconfig.c		\
	picmangeglconfig.h		\
	picmanguiconfig.c			\
	picmanguiconfig.h			\
	picmanpluginconfig.c		\
	picmanpluginconfig.h		\
	picmanrc.c			\
	picmanrc.h			\
	picmanrc-blurbs.h			\
	picmanrc-deserialize.c		\
	picmanrc-deserialize.h		\
	picmanrc-serialize.c		\
	picmanrc-serialize.h		\
	picmanrc-unknown.c		\
	picmanrc-unknown.h		\
	picmanxmlparser.c			\
	picmanxmlparser.h

libappconfig_a_built_sources = \
	config-enums.c

libappconfig_a_SOURCES = \
	$(libappconfig_a_built_sources)	\
	$(libappconfig_a_sources)

EXTRA_PROGRAMS = test-config

#
# unit tests for the PicmanConfig system
#

TESTS = test-config

test_config_DEPENDENCIES = $(picmanconfig_libs)

# We need this due to circular dependencies, see more detailed
# comments about it in app/Makefile.am
test_config_LDFLAGS = \
	-Wl,-u,$(SYMPREFIX)picman_cairo_image_surface_create_buffer	\
	-Wl,-u,$(SYMPREFIX)xcf_init					\
	-Wl,-u,$(SYMPREFIX)picman_pdb_new					\
	-Wl,-u,$(SYMPREFIX)internal_procs_init				\
	-Wl,-u,$(SYMPREFIX)picman_plug_in_manager_restore			\
	-Wl,-u,$(SYMPREFIX)plug_in_icc_profile_apply_rgb		\
	-Wl,-u,$(SYMPREFIX)picman_vectors_undo_get_type			\
	-Wl,-u,$(SYMPREFIX)picman_vectors_mod_undo_get_type		\
	-Wl,-u,$(SYMPREFIX)picman_vectors_prop_undo_get_type		\
	-Wl,-u,$(SYMPREFIX)picman_image_map_config_get_type		\
	-Wl,-u,$(SYMPREFIX)erode_region

test_config_LDADD = \
	../xcf/libappxcf.a			\
	../pdb/libappinternal-procs.a		\
	../pdb/libapppdb.a			\
	../plug-in/libappplug-in.a		\
	../vectors/libappvectors.a		\
	../core/libappcore.a			\
	../file/libappfile.a			\
	../text/libapptext.a			\
	../paint/libapppaint.a			\
	../gegl/libappgegl.a			\
	../operations/libappoperations.a	\
	libappconfig.a				\
	../picman-debug.o				\
	../picman-log.o				\
	$(libpicmanmodule)			\
	$(libpicmancolor)				\
	$(libpicmanthumb)				\
	$(libpicmanmath)				\
	$(libpicmanconfig)			\
	$(libpicmanbase)				\
	$(PANGOCAIRO_LIBS)			\
	$(GDK_PIXBUF_LIBS)			\
	$(GEGL_LIBS)				\
	$(GLIB_LIBS)

CLEANFILES = $(EXTRA_PROGRAMS) foorc

#
# rules to generate built sources
#
# setup autogeneration dependencies
gen_sources = xgen-dec
CLEANFILES += $(gen_sources)

config-enums.c: $(srcdir)/config-enums.h $(PICMAN_MKENUMS)
	$(PICMAN_MKENUMS) \
		--fhead "#include \"config.h\"\n#include <glib-object.h>\n#include \"libpicmanbase/picmanbase.h\"\n#include \"config-enums.h\"\n#include\"picman-intl.h\"" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static const G@Type@Value values[] =\n  {" \
		--vprod "    { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "    { 0, NULL, NULL }\n  };\n" \
		--dhead "  static const Picman@Type@Desc descs[] =\n  {" \
		--dprod "    { @VALUENAME@, @valuedesc@, @valuehelp@ }," \
		--dtail "    { 0, NULL, NULL }\n  };\n\n  static GType type = 0;\n\n  if (G_UNLIKELY (! type))\n    {\n      type = g_@type@_register_static (\"@EnumName@\", values);\n      picman_type_set_translation_context (type, \"@enumnick@\");\n      picman_@type@_set_value_descriptions (type, descs);\n    }\n\n  return type;\n}\n" \
		$(srcdir)/config-enums.h > xgen-dec \
	&& cp xgen-dec $(@F) \
	&& rm -f xgen-dec

