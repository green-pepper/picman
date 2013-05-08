## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Picman-Plug-In\"

INCLUDES = \
	-I$(top_builddir)	\
	-I$(top_srcdir)		\
	-I$(top_builddir)/app	\
	-I$(top_srcdir)/app	\
	$(CAIRO_CFLAGS)		\
	$(GEGL_CFLAGS)		\
	$(GDK_PIXBUF_CFLAGS)	\
	-I$(includedir)

noinst_LIBRARIES = libappplug-in.a

libappplug_in_a_SOURCES = \
	plug-in-enums.c 			\
	plug-in-enums.h 			\
	plug-in-types.h 			\
	\
	picmanenvirontable.c			\
	picmanenvirontable.h			\
	picmaninterpreterdb.c			\
	picmaninterpreterdb.h			\
	picmanplugindebug.c			\
	picmanplugindebug.h			\
	picmanplugin.c				\
	picmanplugin.h				\
	picmanplugin-cleanup.c			\
	picmanplugin-cleanup.h			\
	picmanplugin-context.c			\
	picmanplugin-context.h			\
	picmanplugin-message.c			\
	picmanplugin-message.h			\
	picmanplugin-progress.c			\
	picmanplugin-progress.h			\
	picmanplugindef.c				\
	picmanplugindef.h				\
	picmanpluginerror.c 			\
	picmanpluginerror.h 			\
	picmanpluginmanager.c			\
	picmanpluginmanager.h			\
	picmanpluginmanager-call.c		\
	picmanpluginmanager-call.h		\
	picmanpluginmanager-data.c		\
	picmanpluginmanager-data.h		\
	picmanpluginmanager-file.c		\
	picmanpluginmanager-file.h		\
	picmanpluginmanager-help-domain.c		\
	picmanpluginmanager-help-domain.h		\
	picmanpluginmanager-history.c		\
	picmanpluginmanager-history.h		\
	picmanpluginmanager-locale-domain.c	\
	picmanpluginmanager-locale-domain.h	\
	picmanpluginmanager-menu-branch.c		\
	picmanpluginmanager-menu-branch.h		\
	picmanpluginmanager-query.c		\
	picmanpluginmanager-query.h		\
	picmanpluginmanager-restore.c		\
	picmanpluginmanager-restore.h		\
	picmanpluginprocedure.c			\
	picmanpluginprocedure.h			\
	picmanpluginprocframe.c			\
	picmanpluginprocframe.h			\
	picmanpluginshm.c				\
	picmanpluginshm.h				\
	picmantemporaryprocedure.c		\
	picmantemporaryprocedure.h		\
	\
	plug-in-menu-path.c			\
	plug-in-menu-path.h			\
	plug-in-params.c			\
	plug-in-params.h			\
	plug-in-rc.c				\
	plug-in-rc.h				\
	\
	plug-in-icc-profile.c			\
	plug-in-icc-profile.h

#
# rules to generate built sources
#
# setup autogeneration dependencies
gen_sources = xgen-bec
CLEANFILES = $(EXTRA_PROGRAMS) $(gen_sources)

plug-in-enums.c: $(srcdir)/plug-in-enums.h $(PICMAN_MKENUMS)
	$(PICMAN_MKENUMS) \
		--fhead "#include \"config.h\"\n#include <glib-object.h>\n#include \"libpicmanbase/picmanbase.h\"\n#include \"plug-in-enums.h\"\n#include \"picman-intl.h\"" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static const G@Type@Value values[] =\n  {" \
		--vprod "    { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "    { 0, NULL, NULL }\n  };\n" \
		--dhead "  static const Picman@Type@Desc descs[] =\n  {" \
		--dprod "    { @VALUENAME@, @valuedesc@, @valuehelp@ }," \
		--dtail "    { 0, NULL, NULL }\n  };\n\n  static GType type = 0;\n\n  if (G_UNLIKELY (! type))\n    {\n      type = g_@type@_register_static (\"@EnumName@\", values);\n      picman_type_set_translation_context (type, \"@enumnick@\");\n      picman_@type@_set_value_descriptions (type, descs);\n    }\n\n  return type;\n}\n" \
		$(srcdir)/plug-in-enums.h > xgen-bec \
	&& cp xgen-bec $(@F) \
	&& rm -f xgen-bec
