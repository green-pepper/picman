## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Picman-GEGL\"

INCLUDES = \
	-I$(top_builddir)	\
	-I$(top_srcdir)		\
	-I$(top_builddir)/app	\
	-I$(top_srcdir)/app	\
	$(CAIRO_CFLAGS)		\
	$(GEGL_CFLAGS)		\
	$(GDK_PIXBUF_CFLAGS)	\
	-I$(includedir)

noinst_LIBRARIES = libappgegl.a

libappgegl_a_sources = \
	picman-gegl-enums.h		\
	picman-gegl-types.h		\
	picman-babl.c			\
	picman-babl.h			\
	picman-babl-compat.c		\
	picman-babl-compat.h		\
	picman-gegl.c			\
	picman-gegl.h			\
	picman-gegl-apply-operation.c	\
	picman-gegl-apply-operation.h	\
	picman-gegl-config-proxy.c	\
	picman-gegl-config-proxy.h	\
	picman-gegl-loops.c		\
	picman-gegl-loops.h		\
	picman-gegl-mask.c		\
	picman-gegl-mask.h		\
	picman-gegl-mask-combine.c	\
	picman-gegl-mask-combine.h	\
	picman-gegl-nodes.c		\
	picman-gegl-nodes.h		\
	picman-gegl-tile-compat.c		\
	picman-gegl-tile-compat.h		\
	picman-gegl-utils.c		\
	picman-gegl-utils.h		\
	picmanapplicator.c		\
	picmanapplicator.h		\
	picmantilehandlerprojection.c	\
	picmantilehandlerprojection.h

libappgegl_a_built_sources = picman-gegl-enums.c

libappgegl_a_SOURCES = $(libappgegl_a_built_sources) $(libappgegl_a_sources)

#
# rules to generate built sources
#
# setup autogeneration dependencies
gen_sources = xgen-tec
CLEANFILES = $(gen_sources)

picman-gegl-enums.c: $(srcdir)/picman-gegl-enums.h $(PICMAN_MKENUMS)
	$(PICMAN_MKENUMS) \
		--fhead "#include \"config.h\"\n#include <glib-object.h>\n#include \"libpicmanbase/picmanbase.h\"\n#include \"core/core-enums.h\"\n#include \"picman-gegl-enums.h\"\n#include \"picman-intl.h\"" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static const G@Type@Value values[] =\n  {" \
		--vprod "    { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "    { 0, NULL, NULL }\n  };\n" \
		--dhead "  static const Picman@Type@Desc descs[] =\n  {" \
		--dprod "    { @VALUENAME@, @valuedesc@, @valuehelp@ }," \
		--dtail "    { 0, NULL, NULL }\n  };\n\n  static GType type = 0;\n\n  if (G_UNLIKELY (! type))\n    {\n      type = g_@type@_register_static (\"@EnumName@\", values);\n      picman_type_set_translation_context (type, \"@enumnick@\");\n      picman_@type@_set_value_descriptions (type, descs);\n    }\n\n  return type;\n}\n" \
		$(srcdir)/picman-gegl-enums.h > xgen-tec \
	&& cp xgen-tec $(@F) \
	&& rm -f xgen-tec
