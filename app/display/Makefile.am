## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Picman-Display\"

INCLUDES = \
	-I$(top_builddir)	\
	-I$(top_srcdir)		\
	-I$(top_builddir)/app	\
	-I$(top_srcdir)/app	\
	$(GEGL_CFLAGS)		\
	$(GTK_CFLAGS)		\
	-I$(includedir)

noinst_LIBRARIES = libappdisplay.a

libappdisplay_a_sources = \
	display-enums.h				\
	display-types.h				\
	picmancanvas.c				\
	picmancanvas.h				\
	picmancanvas-style.c			\
	picmancanvas-style.h			\
	picmancanvasarc.c				\
	picmancanvasarc.h				\
	picmancanvasboundary.c			\
	picmancanvasboundary.h			\
	picmancanvascorner.c			\
	picmancanvascorner.h			\
	picmancanvascursor.c			\
	picmancanvascursor.h			\
	picmancanvasgrid.c			\
	picmancanvasgrid.h			\
	picmancanvasgroup.c			\
	picmancanvasgroup.h			\
	picmancanvasguide.c			\
	picmancanvasguide.h			\
	picmancanvashandle.c			\
	picmancanvashandle.h			\
	picmancanvasitem.c			\
	picmancanvasitem.h			\
	picmancanvasitem-utils.c			\
	picmancanvasitem-utils.h			\
	picmancanvaslayerboundary.c		\
	picmancanvaslayerboundary.h		\
	picmancanvasline.c			\
	picmancanvasline.h			\
	picmancanvaspassepartout.c		\
	picmancanvaspassepartout.h		\
	picmancanvaspath.c			\
	picmancanvaspath.h			\
	picmancanvaspen.c				\
	picmancanvaspen.h				\
	picmancanvaspolygon.c			\
	picmancanvaspolygon.h			\
	picmancanvasprogress.c			\
	picmancanvasprogress.h			\
	picmancanvasproxygroup.c			\
	picmancanvasproxygroup.h			\
	picmancanvasrectangle.c			\
	picmancanvasrectangle.h			\
	picmancanvasrectangleguides.c		\
	picmancanvasrectangleguides.h		\
	picmancanvassamplepoint.c			\
	picmancanvassamplepoint.h			\
	picmancanvastextcursor.c			\
	picmancanvastextcursor.h			\
	picmancanvastransformguides.c		\
	picmancanvastransformguides.h		\
	picmancanvastransformpreview.c		\
	picmancanvastransformpreview.h		\
	picmancursorview.c			\
	picmancursorview.h			\
	picmandisplay.c				\
	picmandisplay.h				\
	picmandisplay-foreach.c			\
	picmandisplay-foreach.h			\
	picmandisplay-handlers.c			\
	picmandisplay-handlers.h			\
	picmandisplayshell.c			\
	picmandisplayshell.h			\
	picmandisplayshell-appearance.c		\
	picmandisplayshell-appearance.h		\
	picmandisplayshell-autoscroll.c		\
	picmandisplayshell-autoscroll.h		\
	picmandisplayshell-callbacks.c		\
	picmandisplayshell-callbacks.h		\
	picmandisplayshell-close.c		\
	picmandisplayshell-close.h		\
	picmandisplayshell-cursor.c		\
	picmandisplayshell-cursor.h		\
	picmandisplayshell-dnd.c			\
	picmandisplayshell-dnd.h			\
	picmandisplayshell-draw.c			\
	picmandisplayshell-draw.h			\
	picmandisplayshell-expose.c		\
	picmandisplayshell-expose.h		\
	picmandisplayshell-grab.c			\
	picmandisplayshell-grab.h			\
	picmandisplayshell-handlers.c		\
	picmandisplayshell-handlers.h		\
	picmandisplayshell-filter.c		\
	picmandisplayshell-filter.h		\
	picmandisplayshell-filter-dialog.c	\
	picmandisplayshell-filter-dialog.h	\
	picmandisplayshell-layer-select.c		\
	picmandisplayshell-layer-select.h		\
	picmandisplayshell-icon.c			\
	picmandisplayshell-icon.h			\
	picmandisplayshell-items.c		\
	picmandisplayshell-items.h		\
	picmandisplayshell-progress.c		\
	picmandisplayshell-progress.h		\
	picmandisplayshell-render.c		\
	picmandisplayshell-render.h		\
	picmandisplayshell-rotate.c		\
	picmandisplayshell-rotate.h		\
	picmandisplayshell-rotate-dialog.c	\
	picmandisplayshell-rotate-dialog.h	\
	picmandisplayshell-scale.c		\
	picmandisplayshell-scale.h		\
	picmandisplayshell-scale-dialog.c		\
	picmandisplayshell-scale-dialog.h		\
	picmandisplayshell-scroll.c		\
	picmandisplayshell-scroll.h		\
	picmandisplayshell-selection.c		\
	picmandisplayshell-selection.h		\
	picmandisplayshell-title.c		\
	picmandisplayshell-title.h		\
	picmandisplayshell-tool-events.c		\
	picmandisplayshell-tool-events.h		\
	picmandisplayshell-transform.c		\
	picmandisplayshell-transform.h		\
	picmandisplayxfer.c			\
	picmandisplayxfer.h			\
	picmanimagewindow.c			\
	picmanimagewindow.h			\
	picmanmotionbuffer.c			\
	picmanmotionbuffer.h			\
	picmanmultiwindowstrategy.c		\
	picmanmultiwindowstrategy.h		\
	picmannavigationeditor.c			\
	picmannavigationeditor.h			\
	picmanscalecombobox.c			\
	picmanscalecombobox.h			\
	picmansinglewindowstrategy.c		\
	picmansinglewindowstrategy.h		\
	picmanstatusbar.c				\
	picmanstatusbar.h				\
	picmantooldialog.c			\
	picmantooldialog.h

libappdisplay_a_built_sources = display-enums.c

libappdisplay_a_SOURCES = \
	$(libappdisplay_a_built_sources)	\
	$(libappdisplay_a_sources)

#
# rules to generate built sources
#
# setup autogeneration dependencies
gen_sources = xgen-dec
CLEANFILES = $(gen_sources)

display-enums.c: $(srcdir)/display-enums.h $(PICMAN_MKENUMS)
	$(PICMAN_MKENUMS) \
		--fhead "#include \"config.h\"\n#include <glib-object.h>\n#include \"libpicmanbase/picmanbase.h\"\n#include \"display-enums.h\"\n#include\"picman-intl.h\"" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static const G@Type@Value values[] =\n  {" \
		--vprod "    { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "    { 0, NULL, NULL }\n  };\n" \
		--dhead "  static const Picman@Type@Desc descs[] =\n  {" \
		--dprod "    { @VALUENAME@, @valuedesc@, @valuehelp@ }," \
		--dtail "    { 0, NULL, NULL }\n  };\n\n  static GType type = 0;\n\n  if (G_UNLIKELY (! type))\n    {\n      type = g_@type@_register_static (\"@EnumName@\", values);\n      picman_type_set_translation_context (type, \"@enumnick@\");\n      picman_@type@_set_value_descriptions (type, descs);\n    }\n\n  return type;\n}\n" \
		$(srcdir)/display-enums.h > xgen-dec \
	&& cp xgen-dec $(@F) \
	&& rm -f xgen-dec
