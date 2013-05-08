## Process this file with automake to produce Makefile.in

libpicmanbase = $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la
libpicmancolor = $(top_builddir)/libpicmancolor/libpicmancolor-$(PICMAN_API_VERSION).la
libpicmanconfig = $(top_builddir)/libpicmanconfig/libpicmanconfig-$(PICMAN_API_VERSION).la
libpicmanmodule = $(top_builddir)/libpicmanmodule/libpicmanmodule-$(PICMAN_API_VERSION).la
libpicmanwidgets = $(top_builddir)/libpicmanwidgets/libpicmanwidgets-$(PICMAN_API_VERSION).la
libpicman = ./libpicman-$(PICMAN_API_VERSION).la

if PLATFORM_WIN32
no_undefined = -no-undefined
endif

if PLATFORM_OSX
xobjective_c = "-xobjective-c"
framework_cocoa = -framework Cocoa
endif

if OS_WIN32
picman_def = picman.def
picmanui_def = picmanui.def
libpicman_export_symbols = -export-symbols picman.def
libpicmanui_export_symbols = -export-symbols picmanui.def

install-libtool-import-lib:
	$(INSTALL) .libs/libpicman-$(PICMAN_API_VERSION).dll.a $(DESTDIR)$(libdir)
	$(INSTALL) .libs/libpicmanui-$(PICMAN_API_VERSION).dll.a $(DESTDIR)$(libdir)
	$(INSTALL) picman.def $(DESTDIR)$(libdir)
	$(INSTALL) picmanui.def $(DESTDIR)$(libdir)

uninstall-libtool-import-lib:
	-rm $(DESTDIR)$(libdir)/libpicman-$(PICMAN_API_VERSION).dll.a
	-rm $(DESTDIR)$(libdir)/libpicmanui-$(PICMAN_API_VERSION).dll.a
	-rm $(DESTDIR)$(libdir)/picman.def
	-rm $(DESTDIR)$(libdir)/picmanui.def
else
install-libtool-import-lib:
uninstall-libtool-import-lib:
endif

if MS_LIB_AVAILABLE
noinst_DATA = picman-$(PICMAN_API_VERSION).lib picmanui-$(PICMAN_API_VERSION).lib 

install-ms-lib:
	$(INSTALL) picman-$(PICMAN_API_VERSION).lib $(DESTDIR)$(libdir)
	$(INSTALL) picmanui-$(PICMAN_API_VERSION).lib $(DESTDIR)$(libdir)

uninstall-ms-lib:
	-rm $(DESTDIR)$(libdir)/picman-$(PICMAN_API_VERSION).lib
	-rm $(DESTDIR)$(libdir)/picmanui-$(PICMAN_API_VERSION).lib

picman-@PICMAN_API_VERSION@.lib: picman.def
	lib -name:libpicman-$(PICMAN_API_VERSION)-@LT_CURRENT_MINUS_AGE@.dll -def:picman.def -out:$@

picmanui-@PICMAN_API_VERSION@.lib: picmanui.def
	lib -name:libpicmanui-$(PICMAN_API_VERSION)-@LT_CURRENT_MINUS_AGE@.dll -def:picmanui.def -out:$@

else
install-ms-lib:
uninstall-ms-lib:
endif

picmanincludedir = $(includedir)/picman-$(PICMAN_API_VERSION)/libpicman

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"LibPicman\"	\
	-DPICMAN_COMPILATION

INCLUDES = \
	-I$(top_srcdir)	\
	$(GTK_CFLAGS)	\
	$(GEGL_CFLAGS)	\
	-I$(includedir)	\
	$(xobjective_c)

lib_LTLIBRARIES = libpicman-@PICMAN_API_VERSION@.la libpicmanui-@PICMAN_API_VERSION@.la

PDB_WRAPPERS_C = \
	picman_pdb.c			\
	picmanbrush_pdb.c			\
	picmanbrushes_pdb.c		\
	picmanbrushselect_pdb.c		\
	picmanbuffer_pdb.c		\
	picmanchannel_pdb.c		\
	picmancolor_pdb.c			\
	picmancontext_pdb.c		\
	picmanconvert_pdb.c		\
	picmandisplay_pdb.c		\
	picmandrawable_pdb.c		\
	picmandrawabletransform_pdb.c	\
	picmandynamics_pdb.c		\
	picmanedit_pdb.c			\
	picmanfileops_pdb.c		\
	picmanfloatingsel_pdb.c		\
	picmanfonts_pdb.c			\
	picmanfontselect_pdb.c		\
	picmanpicmanrc_pdb.c		\
	picmangradient_pdb.c		\
	picmangradients_pdb.c		\
	picmangradientselect_pdb.c	\
	picmanguides_pdb.c		\
	picmangrid_pdb.c			\
	picmanhelp_pdb.c			\
	picmanimage_pdb.c			\
	picmanimageselect_pdb.c		\
	picmanitem_pdb.c			\
	picmanitemtransform_pdb.c		\
	picmanlayer_pdb.c			\
	picmanmessage_pdb.c		\
	picmanpainttools_pdb.c		\
	picmanpalette_pdb.c		\
	picmanpalettes_pdb.c		\
	picmanpaletteselect_pdb.c		\
	picmanpaths_pdb.c			\
	picmanpattern_pdb.c		\
	picmanpatterns_pdb.c		\
	picmanpatternselect_pdb.c		\
	picmanplugin_pdb.c		\
	picmanproceduraldb_pdb.c		\
	picmanprogress_pdb.c		\
	picmanselection_pdb.c		\
	picmanselectiontools_pdb.c	\
	picmantextlayer_pdb.c		\
	picmantexttool_pdb.c		\
	picmantransformtools_pdb.c	\
	picmanundo_pdb.c			\
	picmanunit_pdb.c			\
	picmanvectors_pdb.c

PDB_WRAPPERS_H = \
	picman_pdb_headers.h		\
	picman_pdb.h			\
	picmanbrush_pdb.h			\
	picmanbrushes_pdb.h		\
	picmanbrushselect_pdb.h		\
	picmanbuffer_pdb.h		\
	picmanchannel_pdb.h		\
	picmancolor_pdb.h			\
	picmancontext_pdb.h		\
	picmanconvert_pdb.h		\
	picmandisplay_pdb.h		\
	picmandrawable_pdb.h		\
	picmandrawabletransform_pdb.h	\
	picmandynamics_pdb.h		\
	picmanedit_pdb.h			\
	picmanfileops_pdb.h		\
	picmanfloatingsel_pdb.h		\
	picmanfonts_pdb.h			\
	picmanfontselect_pdb.h		\
	picmanpicmanrc_pdb.h		\
	picmangradient_pdb.h		\
	picmangradients_pdb.h		\
	picmangradientselect_pdb.h	\
	picmangrid_pdb.h			\
	picmanguides_pdb.h		\
	picmanhelp_pdb.h			\
	picmanimage_pdb.h			\
	picmanimageselect_pdb.h		\
	picmanitem_pdb.h			\
	picmanitemtransform_pdb.h		\
	picmanlayer_pdb.h			\
	picmanmessage_pdb.h		\
	picmanpainttools_pdb.h		\
	picmanpalette_pdb.h		\
	picmanpalettes_pdb.h		\
	picmanpaletteselect_pdb.h		\
	picmanpaths_pdb.h			\
	picmanpattern_pdb.h		\
	picmanpatterns_pdb.h		\
	picmanpatternselect_pdb.h		\
	picmanplugin_pdb.h		\
	picmanproceduraldb_pdb.h		\
	picmanprogress_pdb.h		\
	picmanselection_pdb.h		\
	picmanselectiontools_pdb.h	\
	picmantextlayer_pdb.h		\
	picmantexttool_pdb.h		\
	picmantransformtools_pdb.h	\
	picmanundo_pdb.h			\
	picmanunit_pdb.h			\
	picmanvectors_pdb.h

libpicman_sources = \
	picman.c			\
	picman.h			\
	picmancompat.h		\
	picmantypes.h		\
	picmanenums.h		\
	${PDB_WRAPPERS_C}	\
	${PDB_WRAPPERS_H}	\
	picmanbrushes.c		\
	picmanbrushes.h		\
	picmanbrushselect.c	\
	picmanbrushselect.h	\
	picmanchannel.c		\
	picmanchannel.h		\
	picmandrawable.c		\
	picmandrawable.h		\
	picmanfontselect.c	\
	picmanfontselect.h	\
	picmanpicmanrc.c		\
	picmanpicmanrc.h		\
	picmangradients.c		\
	picmangradients.h		\
	picmangradientselect.c	\
	picmangradientselect.h	\
	picmanimage.c		\
	picmanimage.h		\
	picmanlayer.c		\
	picmanlayer.h		\
	picmanpalette.c		\
	picmanpalette.h		\
	picmanpalettes.c		\
	picmanpalettes.h		\
	picmanpaletteselect.c	\
	picmanpaletteselect.h	\
	picmanpatterns.c		\
	picmanpatterns.h		\
	picmanpatternselect.c	\
	picmanpatternselect.h	\
	picmanpixbuf.c		\
	picmanpixbuf.h		\
	picmanpixelfetcher.c	\
	picmanpixelfetcher.h	\
	picmanpixelrgn.c		\
	picmanpixelrgn.h		\
	picmanplugin.c		\
	picmanplugin.h		\
	picmanproceduraldb.c	\
	picmanproceduraldb.h	\
	picmanprogress.c		\
	picmanprogress.h		\
	picmanregioniterator.c	\
	picmanregioniterator.h	\
	picmanselection.c		\
	picmanselection.h		\
	picmantile.c		\
	picmantile.h		\
	picmantilebackendplugin.c \
	picmantilebackendplugin.h \
	picmanunitcache.c		\
	picmanunitcache.h		\
	picmanvectors.c		\
	picmanvectors.h		\
	stdplugins-intl.h	\
	libpicman-intl.h

libpicman_built_sources = \
	picmanenums.c

libpicman_extra_sources = picmanenums.c.tail

libpicman_@PICMAN_API_VERSION@_la_SOURCES = \
	$(libpicman_built_sources)	\
	$(libpicman_sources)

libpicmanui_sources = \
	picmanui.c			\
	picmanui.h			\
	picmanuitypes.h			\
	picmanaspectpreview.c     	\
	picmanaspectpreview.h     	\
	picmanbrushmenu.c			\
	picmanbrushmenu.h			\
	picmanbrushselectbutton.c		\
	picmanbrushselectbutton.h		\
	picmandrawablepreview.c		\
	picmandrawablepreview.h		\
	picmanexport.c			\
	picmanexport.h			\
	picmanfontmenu.c			\
	picmanfontmenu.h			\
	picmanfontselectbutton.c		\
	picmanfontselectbutton.h		\
	picmangradientmenu.c 		\
	picmangradientmenu.h 		\
	picmangradientselectbutton.c	\
	picmangradientselectbutton.h	\
	picmanimagecombobox.c		\
	picmanimagecombobox.h		\
	picmanitemcombobox.c		\
	picmanitemcombobox.h		\
	picmanmenu.c			\
	picmanmenu.h			\
	picmanpalettemenu.c		\
	picmanpalettemenu.h		\
	picmanpaletteselectbutton.c	\
	picmanpaletteselectbutton.h	\
	picmanpatternmenu.c		\
	picmanpatternmenu.h		\
	picmanpatternselectbutton.c	\
	picmanpatternselectbutton.h	\
	picmanprocbrowserdialog.c		\
	picmanprocbrowserdialog.h		\
	picmanprocview.c			\
	picmanprocview.h			\
	picmanprogressbar.c		\
	picmanprogressbar.h		\
	picmanselectbutton.c		\
	picmanselectbutton.h		\
	picmanzoompreview.c		\
	picmanzoompreview.h

libpicmanui_built_sources = \
	picmanuimarshal.c			\
	picmanuimarshal.h

libpicmanui_extra_sources = picmanuimarshal.list

libpicmanui_@PICMAN_API_VERSION@_la_SOURCES = \
	$(libpicmanui_built_sources)	\
	$(libpicmanui_sources)

picmaninclude_HEADERS = \
	picman.h				\
	picmancompat.h			\
	picmantypes.h			\
	picmanenums.h			\
	${PDB_WRAPPERS_H}		\
	picmanbrushes.h			\
	picmanbrushselect.h		\
	picmanchannel.h			\
	picmandrawable.h			\
	picmanfontselect.h		\
	picmanpicmanrc.h			\
	picmangradients.h			\
	picmangradientselect.h		\
	picmanimage.h			\
	picmanlayer.h			\
	picmanpalette.h			\
	picmanpalettes.h			\
	picmanpaletteselect.h		\
	picmanpatterns.h			\
	picmanpatternselect.h		\
	picmanpixelfetcher.h		\
	picmanpixelrgn.h			\
	picmanplugin.h			\
	picmanproceduraldb.h		\
	picmanprogress.h			\
	picmanregioniterator.h		\
	picmanselection.h			\
	picmantile.h			\
	picmanvectors.h			\
	\
	picmanui.h			\
	picmanuitypes.h			\
	picmanaspectpreview.h		\
	picmanbrushmenu.h			\
	picmanbrushselectbutton.h		\
	picmandrawablepreview.h   	\
	picmanexport.h			\
	picmanfontmenu.h			\
	picmanfontselectbutton.h		\
	picmangradientmenu.h		\
	picmangradientselectbutton.h	\
	picmanimagecombobox.h		\
	picmanitemcombobox.h		\
	picmanmenu.h			\
	picmanpalettemenu.h		\
	picmanpaletteselectbutton.h	\
	picmanpatternmenu.h		\
	picmanpatternselectbutton.h	\
	picmanpixbuf.h			\
	picmanprocbrowserdialog.h		\
	picmanprocview.h			\
	picmanprogressbar.h		\
	picmanselectbutton.h		\
	picmanzoompreview.h

libpicman_@PICMAN_API_VERSION@_la_LDFLAGS = \
	-version-info $(LT_VERSION_INFO)	\
	$(no_undefined)				\
	$(libpicman_export_symbols)		\
	$(framework_cocoa)

libpicman_@PICMAN_API_VERSION@_la_LIBADD = \
	$(libpicmanconfig)	\
	$(libpicmancolor)		\
	$(libpicmanbase)		\
	$(CAIRO_LIBS)		\
	$(GEGL_LIBS)		\
	$(GDK_PIXBUF_LIBS)	\
	$(RT_LIBS)

libpicman_@PICMAN_API_VERSION@_la_DEPENDENCIES = \
	$(picman_def)		\
	$(libpicmanconfig)	\
	$(libpicmancolor)		\
	$(libpicmanbase)

libpicmanui_@PICMAN_API_VERSION@_la_LDFLAGS = \
	-version-info $(LT_VERSION_INFO)	\
	$(no_undefined)				\
	$(libpicmanui_export_symbols)		\
	$(framework_cocoa)

libpicmanui_@PICMAN_API_VERSION@_la_LIBADD = \
	$(libpicman)		\
	$(libpicmanwidgets)	\
	$(libpicmancolor)		\
	$(libpicmanbase)		\
	$(libpicmanmodule)	\
	$(GTK_LIBS)		\
	$(RT_LIBS)

libpicmanui_@PICMAN_API_VERSION@_la_DEPENDENCIES = \
	$(picmanui_def)		\
	$(libpicman)		\
	$(libpicmanwidgets)	\
	$(libpicmanbase)

EXTRA_DIST = \
	COPYING				\
	picman.def			\
	picmanui.def			\
	$(libpicman_extra_sources)	\
	$(libpicmanui_extra_sources)

install-data-local: install-ms-lib install-libtool-import-lib

uninstall-local: uninstall-ms-lib uninstall-libtool-import-lib

#
# rules to generate built sources
#
# setup autogeneration dependencies
gen_sources = xgen-cec xgen-umh xgen-umc
CLEANFILES = $(gen_sources)

picmanenums.c: $(srcdir)/picmanenums.h $(srcdir)/picmanenums.c.tail $(PICMAN_MKENUMS)
	$(PICMAN_MKENUMS) \
		--fhead "#include \"config.h\"\n#include <glib-object.h>\n#undef PICMAN_DISABLE_DEPRECATED\n#include \"libpicmanbase/picmanbase.h\"\n#include \"picmanenums.h\"" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static const G@Type@Value values[] =\n  {" \
		--vprod "    { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "    { 0, NULL, NULL }\n  };\n" \
		--dhead "  static const Picman@Type@Desc descs[] =\n  {" \
		--dprod "    { @VALUENAME@, @valuedesc@, @valuehelp@ }," \
		--dtail "    { 0, NULL, NULL }\n  };\n\n  static GType type = 0;\n\n  if (G_UNLIKELY (! type))\n    {\n      type = g_@type@_register_static (\"@EnumName@\", values);\n      picman_type_set_translation_domain (type, GETTEXT_PACKAGE \"-libpicman\");\n      picman_type_set_translation_context (type, \"@enumnick@\");\n      picman_@type@_set_value_descriptions (type, descs);\n    }\n\n  return type;\n}\n" \
		$(srcdir)/picmanenums.h > xgen-cec \
	&& cat $(srcdir)/picmanenums.c.tail >> xgen-cec \
	&& cp xgen-cec $(@F) \
	&& rm -f xgen-cec

picmanuimarshal.h: $(srcdir)/picmanuimarshal.list
	$(GLIB_GENMARSHAL) --prefix=_picmanui_marshal $(srcdir)/picmanuimarshal.list --header >> xgen-umh \
	&& (cmp -s xgen-umh $(@F) || cp xgen-umh $(@F)) \
	&& rm -f xgen-umh xgen-umh~

picmanuimarshal.c: picmanuimarshal.h
	echo "#include \"picmanuimarshal.h\"" >> xgen-umc \
	&& $(GLIB_GENMARSHAL) --prefix=_picmanui_marshal $(srcdir)/picmanuimarshal.list --body >> xgen-umc \
	&& cp xgen-umc $(@F) \
	&& rm -f xgen-umc xgen-umc~
