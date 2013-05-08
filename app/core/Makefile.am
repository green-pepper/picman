## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DPICMAN_APP_VERSION=\"$(PICMAN_APP_VERSION)\"	\
	-DG_LOG_DOMAIN=\"Picman-Core\"

INCLUDES = \
	-I$(top_builddir)	\
	-I$(top_srcdir)		\
	-I$(top_builddir)/app	\
	-I$(top_srcdir)/app	\
	$(CAIRO_CFLAGS)		\
	$(GEGL_CFLAGS)		\
	$(GDK_PIXBUF_CFLAGS)	\
	-I$(includedir)

noinst_LIBRARIES = libappcore.a

libappcore_a_sources = \
	core-enums.h				\
	core-types.h				\
	picman.c					\
	picman.h					\
	picman-cairo.c				\
	picman-cairo.h				\
	picman-contexts.c				\
	picman-contexts.h				\
	picman-edit.c				\
	picman-edit.h				\
	picman-gradients.c			\
	picman-gradients.h			\
	picman-gui.c				\
	picman-gui.h				\
	picman-modules.c				\
	picman-modules.h				\
	picman-parasites.c			\
	picman-parasites.h			\
	picman-tags.c				\
	picman-tags.h				\
	picman-templates.c			\
	picman-templates.h			\
	picman-transform-resize.c			\
	picman-transform-resize.h			\
	picman-transform-utils.c			\
	picman-transform-utils.h			\
	picman-units.c				\
	picman-units.h				\
	picman-user-install.c			\
	picman-user-install.h			\
	picman-utils.c				\
	picman-utils.h				\
	picmanarea.c				\
	picmanarea.h				\
	picmanbezierdesc.h			\
	picmanbezierdesc.c			\
	picmanboundary.c				\
	picmanboundary.h				\
	picmanbrush.c				\
	picmanbrush.h				\
	picmanbrush-boundary.c			\
	picmanbrush-boundary.h			\
	picmanbrush-header.h			\
	picmanbrush-load.c			\
	picmanbrush-load.h			\
	picmanbrush-transform.c			\
	picmanbrush-transform.h			\
	picmanbrushcache.c			\
	picmanbrushcache.h			\
	picmanbrushclipboard.c			\
	picmanbrushclipboard.h			\
	picmanbrushgenerated.c			\
	picmanbrushgenerated.h			\
	picmanbrushgenerated-load.c		\
	picmanbrushgenerated-load.h		\
	picmanbrushgenerated-save.c		\
	picmanbrushgenerated-save.h		\
	picmanbrushpipe.c				\
	picmanbrushpipe.h				\
	picmanbrushpipe-load.c			\
	picmanbrushpipe-load.h			\
	picmanbuffer.c				\
	picmanbuffer.h				\
	picmanchannel.c				\
	picmanchannel.h				\
	picmanchannel-combine.c			\
	picmanchannel-combine.h			\
	picmanchannel-select.c			\
	picmanchannel-select.h			\
	picmanchannelpropundo.c			\
	picmanchannelpropundo.h			\
	picmanchannelundo.c			\
	picmanchannelundo.h			\
	picmancontainer.c				\
	picmancontainer.h				\
	picmancontainer-filter.c			\
	picmancontainer-filter.h			\
	picmancontext.c				\
	picmancontext.h				\
	picmancoords.c				\
	picmancoords.h				\
	picmancoords-interpolate.c		\
	picmancoords-interpolate.h		\
	picmancurve.c				\
	picmancurve.h				\
	picmancurve-load.c			\
	picmancurve-load.h			\
	picmancurve-map.c				\
	picmancurve-map.h				\
	picmancurve-save.c			\
	picmancurve-save.h			\
	picmandashpattern.c			\
	picmandashpattern.h			\
	picmandata.c				\
	picmandata.h				\
	picmandatafactory.c			\
	picmandatafactory.h			\
	picmandocumentlist.c			\
	picmandocumentlist.h			\
	picmandrawable.c				\
	picmandrawable.h				\
	picmandrawable-blend.c			\
	picmandrawable-blend.h			\
	picmandrawable-bucket-fill.c		\
	picmandrawable-bucket-fill.h		\
	picmandrawable-combine.c			\
	picmandrawable-combine.h			\
	picmandrawable-equalize.c			\
	picmandrawable-equalize.h			\
	picmandrawable-filter.c			\
	picmandrawable-filter.h			\
	picmandrawable-foreground-extract.c	\
	picmandrawable-foreground-extract.h	\
	picmandrawable-histogram.c		\
	picmandrawable-histogram.h		\
	picmandrawable-levels.c			\
	picmandrawable-levels.h			\
	picmandrawable-offset.c			\
	picmandrawable-offset.h			\
	picmandrawable-operation.c		\
	picmandrawable-operation.h		\
	picmandrawable-preview.c			\
	picmandrawable-preview.h			\
	picmandrawable-private.h			\
	picmandrawable-shadow.c			\
	picmandrawable-shadow.h			\
	picmandrawable-stroke.c			\
	picmandrawable-stroke.h			\
	picmandrawable-transform.c		\
	picmandrawable-transform.h		\
	picmandrawablemodundo.c			\
	picmandrawablemodundo.h			\
	picmandrawablestack.c			\
	picmandrawablestack.h			\
	picmandrawableundo.c			\
	picmandrawableundo.h			\
	picmandynamics.c				\
	picmandynamics.h				\
	picmandynamics-load.c			\
	picmandynamics-load.h			\
	picmandynamics-save.c			\
	picmandynamics-save.h			\
	picmandynamicsoutput.c			\
	picmandynamicsoutput.h			\
	picmanerror.c				\
	picmanerror.h				\
	picmanfilloptions.c			\
	picmanfilloptions.h			\
	picmanfilter.c				\
	picmanfilter.h				\
	picmanfilteredcontainer.c			\
	picmanfilteredcontainer.h			\
	picmanfilterstack.c			\
	picmanfilterstack.h			\
	picmanfloatingselundo.c			\
	picmanfloatingselundo.h			\
	picmangradient.c				\
	picmangradient.h				\
	picmangradient-load.c			\
	picmangradient-load.h			\
	picmangradient-save.c			\
	picmangradient-save.h			\
	picmangrid.c				\
	picmangrid.h				\
	picmangrouplayer.c			\
	picmangrouplayer.h			\
	picmangrouplayerundo.c			\
	picmangrouplayerundo.h			\
	picmanguide.c				\
	picmanguide.h				\
	picmanguideundo.c				\
	picmanguideundo.h				\
	picmanhistogram.c				\
	picmanhistogram.h				\
	picmanidtable.c				\
	picmanidtable.h				\
	picmanimage.c				\
	picmanimage.h				\
	picmanimage-arrange.c			\
	picmanimage-arrange.h			\
	picmanimage-colormap.c			\
	picmanimage-colormap.h			\
	picmanimage-contiguous-region.c		\
	picmanimage-contiguous-region.h		\
	picmanimage-convert-fsdither.h		\
	picmanimage-convert-data.h		\
	picmanimage-convert-precision.c		\
	picmanimage-convert-precision.h		\
	picmanimage-convert-type.c		\
	picmanimage-convert-type.h		\
	picmanimage-crop.c			\
	picmanimage-crop.h			\
	picmanimage-duplicate.c			\
	picmanimage-duplicate.h			\
	picmanimage-flip.c			\
	picmanimage-flip.h			\
	picmanimage-grid.h                	\
	picmanimage-grid.c                	\
	picmanimage-guides.c			\
	picmanimage-guides.h			\
	picmanimage-item-list.c			\
	picmanimage-item-list.h			\
	picmanimage-merge.c			\
	picmanimage-merge.h			\
	picmanimage-new.c				\
	picmanimage-new.h				\
	picmanimage-pick-color.c			\
	picmanimage-pick-color.h			\
	picmanimage-pick-layer.c			\
	picmanimage-pick-layer.h			\
	picmanimage-preview.c			\
	picmanimage-preview.h			\
	picmanimage-private.h			\
	picmanimage-quick-mask.c			\
	picmanimage-quick-mask.h			\
	picmanimage-resize.c			\
	picmanimage-resize.h			\
	picmanimage-rotate.c			\
	picmanimage-rotate.h			\
	picmanimage-sample-points.c		\
	picmanimage-sample-points.h		\
	picmanimage-scale.c			\
	picmanimage-scale.h			\
	picmanimage-snap.c			\
	picmanimage-snap.h			\
	picmanimage-undo.c			\
	picmanimage-undo.h			\
	picmanimage-undo-push.c			\
	picmanimage-undo-push.h			\
	picmanimageundo.c				\
	picmanimageundo.h				\
	picmanimagefile.c				\
	picmanimagefile.h				\
	picmanimagemap.c				\
	picmanimagemap.h				\
	picmanimagemapconfig.c			\
	picmanimagemapconfig.h			\
	picmanitem.c				\
	picmanitem.h				\
	picmanitem-exclusive.c			\
	picmanitem-exclusive.h			\
	picmanitem-linked.c			\
	picmanitem-linked.h			\
	picmanitem-preview.c			\
	picmanitem-preview.h			\
	picmanitempropundo.c			\
	picmanitempropundo.h			\
	picmanitemstack.c				\
	picmanitemstack.h				\
	picmanitemtree.c				\
	picmanitemtree.h				\
	picmanitemundo.c				\
	picmanitemundo.h				\
	picmanlayer.c				\
	picmanlayer.h				\
	picmanlayer-floating-sel.c		\
	picmanlayer-floating-sel.h		\
	picmanlayermask.c				\
	picmanlayermask.h				\
	picmanlayermaskpropundo.c			\
	picmanlayermaskpropundo.h			\
	picmanlayermaskundo.c			\
	picmanlayermaskundo.h			\
	picmanlayerpropundo.c			\
	picmanlayerpropundo.h			\
	picmanlayerundo.c				\
	picmanlayerundo.h				\
	picmanlist.c				\
	picmanlist.h				\
	picmanmaskundo.c				\
	picmanmaskundo.h				\
	picmanobject.c				\
	picmanobject.h				\
	picmanpaintinfo.c				\
	picmanpaintinfo.h				\
	picmanpattern.c				\
	picmanpattern.h				\
	picmanpattern-header.h			\
	picmanpattern-load.c			\
	picmanpattern-load.h			\
	picmanpatternclipboard.c			\
	picmanpatternclipboard.h			\
	picmanpalette.c				\
	picmanpalette.h				\
	picmanpalette-import.c			\
	picmanpalette-import.h			\
	picmanpalette-load.c			\
	picmanpalette-load.h			\
	picmanpalette-save.c			\
	picmanpalette-save.h			\
	picmanparamspecs.c			\
	picmanparamspecs.h			\
	picmanparamspecs-desc.c			\
	picmanparamspecs-desc.h			\
	picmanparamspecs-duplicate.c		\
	picmanparamspecs-duplicate.h		\
	picmanparasitelist.c			\
	picmanparasitelist.h			\
	picmanpdbprogress.c			\
	picmanpdbprogress.h			\
	picmanpickable.c				\
	picmanpickable.h				\
	picmanpickable-auto-shrink.c		\
	picmanpickable-auto-shrink.h		\
	picmanprogress.c				\
	picmanprogress.h				\
	picmanprojectable.c			\
	picmanprojectable.h			\
	picmanprojection.c			\
	picmanprojection.h			\
	picmansamplepoint.c			\
	picmansamplepoint.h			\
	picmansamplepointundo.c			\
	picmansamplepointundo.h			\
	picmanscanconvert.c			\
	picmanscanconvert.h			\
	picmanselection.c				\
	picmanselection.h				\
	picmanstrokeoptions.c			\
	picmanstrokeoptions.h			\
	picmansubprogress.c			\
	picmansubprogress.h			\
	picmantag.c				\
	picmantag.h				\
	picmantagcache.c				\
	picmantagcache.h				\
	picmantagged.c				\
	picmantagged.h				\
	picmantaggedcontainer.c			\
	picmantaggedcontainer.h			\
	picmantempbuf.c				\
	picmantempbuf.h				\
	picmantemplate.c				\
	picmantemplate.h				\
	picmantoolinfo.c				\
	picmantoolinfo.h				\
	picmantooloptions.c			\
	picmantooloptions.h			\
	picmantoolpreset.c			\
	picmantoolpreset.h			\
	picmantoolpreset-load.c			\
	picmantoolpreset-load.h			\
	picmantoolpreset-save.c			\
	picmantoolpreset-save.h			\
	picmantreehandler.c			\
	picmantreehandler.h			\
	picmanunit.c				\
	picmanunit.h				\
	picmanundo.c				\
	picmanundo.h				\
	picmanundostack.c				\
	picmanundostack.h				\
	picmanviewable.c				\
	picmanviewable.h

libappcore_a_built_sources = \
	core-enums.c	\
	picmanmarshal.c	\
	picmanmarshal.h

libappcore_a_extra_sources = \
	picmanmarshal.list

libappcore_a_SOURCES = $(libappcore_a_built_sources) $(libappcore_a_sources)

EXTRA_DIST = \
	$(libappcore_a_extra_sources)

#
# rules to generate built sources
#
# setup autogeneration dependencies
gen_sources = xgen-gmh xgen-gmc xgen-cec
CLEANFILES = $(gen_sources)

picmanmarshal.h: $(srcdir)/picmanmarshal.list
	$(GLIB_GENMARSHAL) --prefix=picman_marshal $(srcdir)/picmanmarshal.list --header >> xgen-gmh \
	&& (cmp -s xgen-gmh $(@F) || cp xgen-gmh $(@F)) \
	&& rm -f xgen-gmh xgen-gmh~

picmanmarshal.c: picmanmarshal.h
	echo "#include \"picmanmarshal.h\"" >> xgen-gmc \
	&& $(GLIB_GENMARSHAL) --prefix=picman_marshal $(srcdir)/picmanmarshal.list --body >> xgen-gmc \
	&& cp xgen-gmc $(@F) \
	&& rm -f xgen-gmc xgen-gmc~

core-enums.c: $(srcdir)/core-enums.h $(PICMAN_MKENUMS)
	$(PICMAN_MKENUMS) \
		--fhead "#include \"config.h\"\n#include <glib-object.h>\n#include \"libpicmanbase/picmanbase.h\"\n#include \"core-enums.h\"\n#include \"picman-intl.h\"" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static const G@Type@Value values[] =\n  {" \
		--vprod "    { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "    { 0, NULL, NULL }\n  };\n" \
		--dhead "  static const Picman@Type@Desc descs[] =\n  {" \
		--dprod "    { @VALUENAME@, @valuedesc@, @valuehelp@ }," \
		--dtail "    { 0, NULL, NULL }\n  };\n\n  static GType type = 0;\n\n  if (G_UNLIKELY (! type))\n    {\n      type = g_@type@_register_static (\"@EnumName@\", values);\n      picman_type_set_translation_context (type, \"@enumnick@\");\n      picman_@type@_set_value_descriptions (type, descs);\n    }\n\n  return type;\n}\n" \
		$(srcdir)/core-enums.h > xgen-cec \
	&& cp xgen-cec $(@F) \
	&& rm -f xgen-cec
