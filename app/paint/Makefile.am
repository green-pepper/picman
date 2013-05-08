## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Picman-Paint\"

INCLUDES = \
	-I$(top_builddir)	\
	-I$(top_srcdir)		\
	-I$(top_builddir)/app	\
	-I$(top_srcdir)/app	\
	$(CAIRO_CFLAGS)		\
	$(GEGL_CFLAGS)		\
	$(GDK_PIXBUF_CFLAGS)	\
	-I$(includedir)

noinst_LIBRARIES = libapppaint.a

libapppaint_a_sources = \
	paint-enums.h			\
	paint-types.h			\
	picman-paint.c			\
	picman-paint.h			\
	picmanairbrush.c			\
	picmanairbrush.h			\
	picmanairbrushoptions.c		\
	picmanairbrushoptions.h		\
	picmanbrushcore.c			\
	picmanbrushcore.h			\
	picmanbrushcore-kernels.h		\
	picmanclone.c			\
	picmanclone.h			\
	picmancloneoptions.c		\
	picmancloneoptions.h		\
	picmanconvolve.c			\
	picmanconvolve.h			\
	picmanconvolveoptions.c		\
	picmanconvolveoptions.h		\
	picmandodgeburn.c			\
	picmandodgeburn.h			\
	picmandodgeburnoptions.c		\
	picmandodgeburnoptions.h		\
	picmaneraser.c			\
	picmaneraser.h			\
	picmaneraseroptions.c		\
	picmaneraseroptions.h		\
	picmanheal.c			\
	picmanheal.h			\
	picmanink.c			\
	picmanink.h			\
	picmanink-blob.c			\
	picmanink-blob.h			\
	picmaninkoptions.c		\
	picmaninkoptions.h		\
	picmaninkundo.c			\
	picmaninkundo.h			\
	picmanpaintcore.c			\
	picmanpaintcore.h			\
	picmanpaintcore-stroke.c		\
	picmanpaintcore-stroke.h		\
	picmanpaintcoreundo.c		\
	picmanpaintcoreundo.h		\
	picmanpaintoptions.c		\
	picmanpaintoptions.h		\
	picmanpencil.c			\
	picmanpencil.h			\
	picmanpenciloptions.c		\
	picmanpenciloptions.h		\
	picmanpaintbrush.c		\
	picmanpaintbrush.h		\
	picmanperspectiveclone.c		\
	picmanperspectiveclone.h		\
	picmanperspectivecloneoptions.c	\
	picmanperspectivecloneoptions.h	\
	picmansmudge.c			\
	picmansmudge.h			\
	picmansmudgeoptions.c		\
	picmansmudgeoptions.h		\
	picmansourcecore.c		\
	picmansourcecore.h		\
	picmansourceoptions.c		\
	picmansourceoptions.h

libapppaint_a_built_sources = paint-enums.c

libapppaint_a_SOURCES = $(libapppaint_a_built_sources) $(libapppaint_a_sources)

#
# rules to generate built sources
#
# setup autogeneration dependencies
gen_sources = xgen-pec
CLEANFILES = $(gen_sources)

paint-enums.c: $(srcdir)/paint-enums.h $(PICMAN_MKENUMS)
	$(PICMAN_MKENUMS) \
		--fhead "#include \"config.h\"\n#include <glib-object.h>\n#include \"libpicmanbase/picmanbase.h\"\n#include \"paint-enums.h\"\n#include \"picman-intl.h\"" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static const G@Type@Value values[] =\n  {" \
		--vprod "    { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "    { 0, NULL, NULL }\n  };\n" \
		--dhead "  static const Picman@Type@Desc descs[] =\n  {" \
		--dprod "    { @VALUENAME@, @valuedesc@, @valuehelp@ }," \
		--dtail "    { 0, NULL, NULL }\n  };\n\n  static GType type = 0;\n\n  if (G_UNLIKELY (! type))\n    {\n      type = g_@type@_register_static (\"@EnumName@\", values);\n      picman_type_set_translation_context (type, \"@enumnick@\");\n      picman_@type@_set_value_descriptions (type, descs);\n    }\n\n  return type;\n}\n" \
		$(srcdir)/paint-enums.h > xgen-pec \
	&& cp xgen-pec $(@F) \
	&& rm -f xgen-pec
