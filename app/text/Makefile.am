## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Picman-Text\"

INCLUDES = \
	-I$(top_builddir)	\
	-I$(top_srcdir)		\
	-I$(top_builddir)/app	\
	-I$(top_srcdir)/app	\
	$(GEGL_CFLAGS)		\
	$(PANGOCAIRO_CFLAGS)	\
	$(GDK_PIXBUF_CFLAGS)	\
	-I$(includedir)

noinst_LIBRARIES = libapptext.a

libapptext_a_sources = \
	text-types.h			\
	text-enums.h			\
	picman-fonts.c			\
	picman-fonts.h			\
	picmanfont.c			\
	picmanfont.h			\
	picmanfontlist.c			\
	picmanfontlist.h			\
	picmantext.c			\
	picmantext.h			\
	picmantext-compat.c		\
	picmantext-compat.h		\
	picmantext-parasite.c		\
	picmantext-parasite.h		\
	picmantext-vectors.c		\
	picmantext-vectors.h		\
	picmantext-xlfd.c			\
	picmantext-xlfd.h			\
	picmantextlayer.c			\
	picmantextlayer.h			\
	picmantextlayer-transform.c	\
	picmantextlayer-transform.h	\
	picmantextlayer-xcf.c		\
	picmantextlayer-xcf.h		\
	picmantextlayout.c		\
	picmantextlayout.h		\
	picmantextlayout-render.c		\
	picmantextlayout-render.h		\
	picmantextundo.c			\
	picmantextundo.h

libapptext_a_built_sources = text-enums.c

libapptext_a_SOURCES = $(libapptext_a_built_sources) $(libapptext_a_sources)

#
# rules to generate built sources
#
# setup autogeneration dependencies
gen_sources = xgen-tec
CLEANFILES = $(gen_sources)

text-enums.c: $(srcdir)/text-enums.h $(PICMAN_MKENUMS)
	$(PICMAN_MKENUMS) \
		--fhead "#include \"config.h\"\n#include <glib-object.h>\n#include \"libpicmanbase/picmanbase.h\"\n#include \"text-enums.h\"\n#include \"picman-intl.h\"" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static const G@Type@Value values[] =\n  {" \
		--vprod "    { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "    { 0, NULL, NULL }\n  };\n" \
		--dhead "  static const Picman@Type@Desc descs[] =\n  {" \
		--dprod "    { @VALUENAME@, @valuedesc@, @valuehelp@ }," \
		--dtail "    { 0, NULL, NULL }\n  };\n\n  static GType type = 0;\n\n  if (G_UNLIKELY (! type))\n    {\n      type = g_@type@_register_static (\"@EnumName@\", values);\n      picman_type_set_translation_context (type, \"@enumnick@\");\n      picman_@type@_set_value_descriptions (type, descs);\n    }\n\n  return type;\n}\n" \
		$(srcdir)/text-enums.h > xgen-tec \
	&& cp xgen-tec $(@F) \
	&& rm -f xgen-tec
