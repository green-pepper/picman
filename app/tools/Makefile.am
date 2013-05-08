## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Picman-Tools\"

INCLUDES = \
	-I$(top_builddir)	\
	-I$(top_srcdir)		\
	-I$(top_builddir)/app	\
	-I$(top_srcdir)/app	\
	$(GEGL_CFLAGS)		\
	$(GTK_CFLAGS)		\
	-I$(includedir)

noinst_LIBRARIES = libapptools.a

libapptools_a_sources = \
	tools-enums.h			\
	tools-types.h			\
	picman-tools.c			\
	picman-tools.h			\
	tool_manager.c			\
	tool_manager.h			\
	\
	picmanairbrushtool.c		\
	picmanairbrushtool.h		\
	picmanalignoptions.c		\
	picmanalignoptions.h		\
	picmanaligntool.c			\
	picmanaligntool.h			\
	picmanblendoptions.c		\
	picmanblendoptions.h		\
	picmanblendtool.c			\
	picmanblendtool.h			\
	picmanbrightnesscontrasttool.c	\
	picmanbrightnesscontrasttool.h	\
	picmanbrushtool.c			\
	picmanbrushtool.h			\
	picmanbucketfilloptions.c		\
	picmanbucketfilloptions.h		\
	picmanbucketfilltool.c		\
	picmanbucketfilltool.h		\
	picmanbycolorselecttool.c		\
	picmanbycolorselecttool.h		\
	picmancageoptions.c		\
	picmancageoptions.h		\
	picmancagetool.c			\
	picmancagetool.h			\
	picmanclonetool.c			\
	picmanclonetool.h			\
	picmancoloroptions.c		\
	picmancoloroptions.h		\
	picmancolortool.c			\
	picmancolortool.h			\
	picmancolorbalancetool.c		\
	picmancolorbalancetool.h		\
	picmancolorizetool.c		\
	picmancolorizetool.h		\
	picmancolorpickeroptions.c	\
	picmancolorpickeroptions.h	\
	picmancolorpickertool.c		\
	picmancolorpickertool.h		\
	picmanconvolvetool.c		\
	picmanconvolvetool.h		\
	picmancropoptions.c		\
	picmancropoptions.h		\
	picmancroptool.c			\
	picmancroptool.h			\
	picmancurvestool.c		\
	picmancurvestool.h		\
	picmandesaturatetool.c		\
	picmandesaturatetool.h		\
	picmandodgeburntool.c		\
	picmandodgeburntool.h		\
	picmandrawtool.c			\
	picmandrawtool.h			\
	picmaneditselectiontool.c		\
	picmaneditselectiontool.h		\
	picmanellipseselecttool.c		\
	picmanellipseselecttool.h		\
	picmanerasertool.c		\
	picmanerasertool.h		\
	picmanflipoptions.c		\
	picmanflipoptions.h		\
	picmanfliptool.c			\
	picmanfliptool.h			\
	picmanfreeselecttool.c		\
	picmanfreeselecttool.h		\
	picmanforegroundselectoptions.c	\
	picmanforegroundselectoptions.h	\
	picmanforegroundselecttool.c	\
	picmanforegroundselecttool.h	\
	picmanforegroundselecttoolundo.c	\
	picmanforegroundselecttoolundo.h	\
	picmanfuzzyselecttool.c		\
	picmanfuzzyselecttool.h		\
	picmangegltool.c			\
	picmangegltool.h			\
	picmanhealtool.c			\
	picmanhealtool.h			\
	picmanhistogramoptions.c		\
	picmanhistogramoptions.h		\
	picmanhuesaturationtool.c		\
	picmanhuesaturationtool.h		\
	picmanimagemapoptions.c		\
	picmanimagemapoptions.h		\
	picmanimagemaptool.c		\
	picmanimagemaptool.h		\
	picmanimagemaptool-settings.c	\
	picmanimagemaptool-settings.h	\
	picmaninkoptions-gui.c		\
	picmaninkoptions-gui.h		\
	picmaninktool.c			\
	picmaninktool.h			\
	picmaniscissorsoptions.c		\
	picmaniscissorsoptions.h		\
	picmaniscissorstool.c		\
	picmaniscissorstool.h		\
	picmanlevelstool.c		\
	picmanlevelstool.h		\
	picmanoperationtool.c		\
	picmanoperationtool.h		\
	picmanmagnifyoptions.c		\
	picmanmagnifyoptions.h		\
	picmanmagnifytool.c		\
	picmanmagnifytool.h		\
	picmanmeasureoptions.c		\
	picmanmeasureoptions.h		\
	picmanmeasuretool.c		\
	picmanmeasuretool.h		\
	picmanmoveoptions.c		\
	picmanmoveoptions.h		\
	picmanmovetool.c			\
	picmanmovetool.h			\
	picmanpaintbrushtool.c		\
	picmanpaintbrushtool.h		\
	picmanpaintoptions-gui.c		\
	picmanpaintoptions-gui.h		\
	picmanpainttool.c			\
	picmanpainttool.h			\
	picmanpenciltool.c		\
	picmanpenciltool.h		\
	picmanperspectiveclonetool.c	\
	picmanperspectiveclonetool.h	\
	picmanperspectivetool.c		\
	picmanperspectivetool.h		\
	picmanposterizetool.c		\
	picmanposterizetool.h		\
	picmanrectangleselecttool.c	\
	picmanrectangleselecttool.h	\
	picmanrectangleselectoptions.c	\
	picmanrectangleselectoptions.h	\
	picmanrectangletool.c		\
	picmanrectangletool.h		\
	picmanrectangleoptions.c		\
	picmanrectangleoptions.h		\
	picmanregionselectoptions.c	\
	picmanregionselectoptions.h	\
	picmanregionselecttool.c		\
	picmanregionselecttool.h		\
	picmanrotatetool.c		\
	picmanrotatetool.h		\
	picmanscaletool.c			\
	picmanscaletool.h			\
	picmanselectionoptions.c		\
	picmanselectionoptions.h		\
	picmanselectiontool.c		\
	picmanselectiontool.h		\
	picmansheartool.c			\
	picmansheartool.h			\
	picmansmudgetool.c		\
	picmansmudgetool.h		\
	picmansourcetool.c		\
	picmansourcetool.h		\
	picmantextoptions.c		\
	picmantextoptions.h		\
	picmantexttool.c			\
	picmantexttool.h			\
	picmantexttool-editor.c		\
	picmantexttool-editor.h		\
	picmanthresholdtool.c		\
	picmanthresholdtool.h		\
	picmantool.c			\
	picmantool.h			\
	picmantool-progress.c		\
	picmantool-progress.h		\
	picmantoolcontrol.c		\
	picmantoolcontrol.h		\
	picmantooloptions-gui.c		\
	picmantooloptions-gui.h		\
	picmantransformoptions.c     	\
	picmantransformoptions.h     	\
	picmantransformtool.c		\
	picmantransformtool.h		\
	picmantransformtoolundo.c		\
	picmantransformtoolundo.h		\
	picmanunifiedtransformtool.c	\
	picmanunifiedtransformtool.h	\
	picmanvectoroptions.c		\
	picmanvectoroptions.h		\
	picmanvectortool.c		\
	picmanvectortool.h

libapptools_a_built_sources = tools-enums.c

libapptools_a_SOURCES = $(libapptools_a_built_sources) $(libapptools_a_sources)

#
# rules to generate built sources
#
# setup autogeneration dependencies
gen_sources = xgen-tec
CLEANFILES = $(gen_sources)

tools-enums.c: $(srcdir)/tools-enums.h $(PICMAN_MKENUMS)
	$(PICMAN_MKENUMS) \
		--fhead "#include \"config.h\"\n#include <glib-object.h>\n#include \"libpicmanbase/picmanbase.h\"\n#include \"core/core-enums.h\"\n#include \"tools-enums.h\"\n#include \"picman-intl.h\"" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static const G@Type@Value values[] =\n  {" \
		--vprod "    { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "    { 0, NULL, NULL }\n  };\n" \
		--dhead "  static const Picman@Type@Desc descs[] =\n  {" \
		--dprod "    { @VALUENAME@, @valuedesc@, @valuehelp@ }," \
		--dtail "    { 0, NULL, NULL }\n  };\n\n  static GType type = 0;\n\n  if (G_UNLIKELY (! type))\n    {\n      type = g_@type@_register_static (\"@EnumName@\", values);\n      picman_type_set_translation_context (type, \"@enumnick@\");\n      picman_@type@_set_value_descriptions (type, descs);\n    }\n\n  return type;\n}\n" \
		$(srcdir)/tools-enums.h > xgen-tec \
	&& cp xgen-tec $(@F) \
	&& rm -f xgen-tec
