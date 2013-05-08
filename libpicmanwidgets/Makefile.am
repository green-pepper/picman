## Process this file with automake to produce Makefile.in

libpicmanbase = $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la
libpicmancolor = $(top_builddir)/libpicmancolor/libpicmancolor-$(PICMAN_API_VERSION).la
libpicmanconfig = $(top_builddir)/libpicmanconfig/libpicmanconfig-$(PICMAN_API_VERSION).la

if PLATFORM_WIN32
no_undefined = -no-undefined
endif

if PLATFORM_WIN32
else
libm = -lm
endif

if OS_WIN32
picmanwidgets_def = picmanwidgets.def
libpicmanwidgets_export_symbols = -export-symbols picmanwidgets.def

install-libtool-import-lib:
	$(INSTALL) .libs/libpicmanwidgets-$(PICMAN_API_VERSION).dll.a $(DESTDIR)$(libdir)
	$(INSTALL) picmanwidgets.def $(DESTDIR)$(libdir)

uninstall-libtool-import-lib:
	-rm $(DESTDIR)$(libdir)/libpicmanwidgets-$(PICMAN_API_VERSION).dll.a
	-rm $(DESTDIR)$(libdir)/picmanwidgets.def
else
install-libtool-import-lib:
uninstall-libtool-import-lib:
endif

if MS_LIB_AVAILABLE
noinst_DATA = picmanwidgets-$(PICMAN_API_VERSION).lib

install-ms-lib:
	$(INSTALL) picmanwidgets-$(PICMAN_API_VERSION).lib $(DESTDIR)$(libdir)

uninstall-ms-lib:
	-rm $(DESTDIR)$(libdir)/picmanwidgets-$(PICMAN_API_VERSION).lib

picmanwidgets-@PICMAN_API_VERSION@.lib: picmanwidgets.def
	lib -name:libpicmanwidgets-$(PICMAN_API_VERSION)-@LT_CURRENT_MINUS_AGE@.dll -def:picmanwidgets.def -out:$@

else
install-ms-lib:
uninstall-ms-lib:
endif

libpicmanwidgetsincludedir = $(includedir)/picman-$(PICMAN_API_VERSION)/libpicmanwidgets

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"LibPicmanWidgets\"	\
	-DPICMAN_WIDGETS_COMPILATION

INCLUDES = \
	-I$(top_srcdir)	\
	$(GEGL_CFLAGS)	\
	$(GTK_CFLAGS)	\
	-I$(includedir)

lib_LTLIBRARIES = libpicmanwidgets-@PICMAN_API_VERSION@.la

libpicmanwidgets_sources = \
	picmanbrowser.c			\
	picmanbrowser.h			\
	picmanbutton.c			\
	picmanbutton.h			\
	picmancairo-utils.c		\
	picmancairo-utils.h		\
	picmancellrenderercolor.c		\
	picmancellrenderercolor.h		\
	picmancellrenderertoggle.c	\
	picmancellrenderertoggle.h	\
	picmanchainbutton.c		\
	picmanchainbutton.h		\
	picmancolorarea.c			\
	picmancolorarea.h			\
	picmancolorbutton.c		\
	picmancolorbutton.h		\
	picmancolordisplay.c		\
	picmancolordisplay.h		\
	picmancolordisplaystack.c		\
	picmancolordisplaystack.h		\
	picmancolorhexentry.c		\
	picmancolorhexentry.h		\
	picmancolornotebook.c		\
	picmancolornotebook.h		\
	picmancolorprofilecombobox.c	\
	picmancolorprofilecombobox.h	\
	picmancolorprofilestore-private.h	\
	picmancolorprofilestore.c		\
	picmancolorprofilestore.h		\
	picmancolorscale.c		\
	picmancolorscale.h		\
	picmancolorscales.c		\
	picmancolorscales.h		\
	picmancolorselect.c		\
	picmancolorselect.h		\
	picmancolorselection.c		\
	picmancolorselection.h		\
	picmancolorselector.c		\
	picmancolorselector.h		\
	picmancontroller.c		\
	picmancontroller.h		\
	picmandialog.c			\
	picmandialog.h			\
	picmaneevl.c			\
	picmaneevl.h			\
	picmanenumcombobox.c		\
	picmanenumcombobox.h		\
	picmanenumlabel.c			\
	picmanenumlabel.h			\
	picmanenumstore.c			\
	picmanenumstore.h			\
	picmanenumwidgets.c		\
	picmanenumwidgets.h		\
	picmanfileentry.c			\
	picmanfileentry.h			\
	picmanframe.c			\
	picmanframe.h			\
	picmanhelpui.c			\
	picmanhelpui.h			\
	picmanhintbox.c			\
	picmanhintbox.h			\
	picmanintcombobox.c		\
	picmanintcombobox.h		\
	picmanintstore.c			\
	picmanintstore.h			\
	picmanmemsizeentry.c		\
	picmanmemsizeentry.h		\
	picmannumberpairentry.c		\
	picmannumberpairentry.h		\
	picmanoffsetarea.c		\
	picmanoffsetarea.h		\
	picmanoldwidgets.c		\
	picmanoldwidgets.h		\
	picmanpageselector.c		\
	picmanpageselector.h		\
	picmanpatheditor.c		\
	picmanpatheditor.h		\
	picmanpickbutton.c		\
	picmanpickbutton.h		\
	picmanpixmap.c			\
	picmanpixmap.h			\
	picmanpreview.c			\
	picmanpreview.h			\
	picmanpreviewarea.c		\
	picmanpreviewarea.h		\
	picmanpropwidgets.c		\
	picmanpropwidgets.h		\
	picmanquerybox.c			\
	picmanquerybox.h			\
	picmanruler.c			\
	picmanruler.h			\
	picmanscaleentry.c		\
	picmanscaleentry.h		\
	picmanscrolledpreview.c		\
	picmanscrolledpreview.h		\
	picmansizeentry.c			\
	picmansizeentry.h			\
	picmanstock.c			\
	picmanstock.h			\
	picmanstringcombobox.c		\
	picmanstringcombobox.h		\
	picmanunitcombobox.c		\
	picmanunitcombobox.h		\
	picmanunitmenu.c			\
	picmanunitmenu.h			\
	picmanunitstore.c			\
	picmanunitstore.h			\
	picmanwidgets-error.c		\
	picmanwidgets-error.h		\
	picmanwidgets-private.c		\
	picmanwidgets-private.h		\
	picmanwidgets.c			\
	picmanwidgets.h			\
	picmanwidgetsenums.h		\
	picmanwidgetstypes.h		\
	picmanzoommodel.c			\
	picmanzoommodel.h			\
	picman3migration.c		\
	picman3migration.h

libpicmanwidgets_built_sources = \
	picman-wilber-pixbufs.h	\
	picmanwidgetsenums.c	\
	picmanwidgetsmarshal.c	\
	picmanwidgetsmarshal.h

libpicmanwidgets_extra_sources = picmanwidgetsmarshal.list

libpicmanwidgets_@PICMAN_API_VERSION@_la_SOURCES = \
	$(libpicmanwidgets_built_sources)	\
	$(libpicmanwidgets_sources)

libpicmanwidgetsinclude_HEADERS = \
	picmanbrowser.h			\
	picmanbutton.h			\
	picmancairo-utils.h		\
	picmancellrenderercolor.h		\
	picmancellrenderertoggle.h	\
	picmanchainbutton.h		\
	picmancolorarea.h			\
	picmancolorbutton.h		\
	picmancolordisplay.h		\
	picmancolordisplaystack.h		\
	picmancolorhexentry.h		\
	picmancolornotebook.h		\
	picmancolorprofilecombobox.h	\
	picmancolorprofilestore.h		\
	picmancolorscale.h		\
	picmancolorscales.h		\
	picmancolorselect.h		\
	picmancolorselection.h		\
	picmancolorselector.h		\
	picmancontroller.h		\
	picmandialog.h			\
	picmanenumcombobox.h		\
	picmanenumlabel.h			\
	picmanenumstore.h			\
	picmanenumwidgets.h		\
	picmanfileentry.h			\
	picmanframe.h			\
	picmanhelpui.h			\
	picmanhintbox.h			\
	picmanintcombobox.h		\
	picmanintstore.h			\
	picmanmemsizeentry.h		\
	picmannumberpairentry.h		\
	picmanoffsetarea.h		\
	picmanoldwidgets.h		\
	picmanpageselector.h		\
	picmanpatheditor.h		\
	picmanpickbutton.h		\
	picmanpixmap.h			\
	picmanpreview.h			\
	picmanpreviewarea.h		\
	picmanpropwidgets.h		\
	picmanquerybox.h			\
	picmanruler.h			\
	picmanscaleentry.h		\
	picmanscrolledpreview.h		\
	picmansizeentry.h			\
	picmanstock.h			\
	picmanstringcombobox.h		\
	picmanunitcombobox.h		\
	picmanunitmenu.h			\
	picmanunitstore.h			\
	picmanwidgets-error.h		\
	picmanwidgets.h			\
	picmanwidgetsenums.h		\
	picmanwidgetstypes.h		\
	picmanzoommodel.h			\
	picman3migration.h

libpicmanwidgets_@PICMAN_API_VERSION@_la_LDFLAGS = \
	-version-info $(LT_VERSION_INFO)\
	$(no_undefined) 		\
	$(libpicmanwidgets_export_symbols)

libpicmanwidgets_@PICMAN_API_VERSION@_la_LIBADD = \
	$(libpicmancolor)		\
	$(libpicmanconfig)	\
	$(libpicmanbase)		\
	$(GEGL_LIBS)		\
	$(GTK_LIBS)		\
	$(libm)

libpicmanwidgets_@PICMAN_API_VERSION@_la_DEPENDENCIES = $(picmanwidgets_def)


## Wilber icons for the window managaer, compiled in

WILBER_IMAGES = \
	wilber-16.png	\
	wilber-32.png	\
	wilber-48.png	\
	wilber-64.png

WILBER_VARIABLES = \
	wilber_16  $(srcdir)/wilber-16.png	\
	wilber_32  $(srcdir)/wilber-32.png	\
	wilber_48  $(srcdir)/wilber-48.png	\
	wilber_64  $(srcdir)/wilber-64.png


EXTRA_DIST = \
	picmanwidgets.def			\
	$(libpicmanwidgets_extra_sources)	\
	$(WILBER_IMAGES)


picmanwidgets-private.lo: picman-wilber-pixbufs.h


#
# rules to generate built sources
#
# setup autogeneration dependancies
gen_sources = xgen-wec xgen-wmh xgen-wmc picman-wilber-pixbufs.h
CLEANFILES = $(gen_sources)

picmanwidgetsenums.c: $(srcdir)/picmanwidgetsenums.h $(PICMAN_MKENUMS)
	$(AM_V_GEN) $(PICMAN_MKENUMS) \
		--fhead "#include \"config.h\"\n#include <glib-object.h>\n#include \"libpicmanbase/picmanbase.h\"\n#include \"picmanwidgetsenums.h\"\n#include \"libpicman/libpicman-intl.h\"" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static const G@Type@Value values[] =\n  {" \
		--vprod "    { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "    { 0, NULL, NULL }\n  };\n" \
		--dhead "  static const Picman@Type@Desc descs[] =\n  {" \
		--dprod "    { @VALUENAME@, @valuedesc@, @valuehelp@ }," \
		--dtail "    { 0, NULL, NULL }\n  };\n\n  static GType type = 0;\n\n  if (G_UNLIKELY (! type))\n    {\n      type = g_@type@_register_static (\"@EnumName@\", values);\n      picman_type_set_translation_domain (type, GETTEXT_PACKAGE \"-libpicman\");\n      picman_type_set_translation_context (type, \"@enumnick@\");\n      picman_@type@_set_value_descriptions (type, descs);\n    }\n\n  return type;\n}\n" \
		$(srcdir)/picmanwidgetsenums.h > xgen-wec \
	&& cp xgen-wec $(@F) \
	&& rm -f xgen-wec

picmanwidgetsmarshal.h: $(srcdir)/picmanwidgetsmarshal.list
	$(AM_V_GEN) $(GLIB_GENMARSHAL) --prefix=_picman_widgets_marshal $(srcdir)/picmanwidgetsmarshal.list --header >> xgen-wmh \
	&& (cmp -s xgen-wmh $(@F) || cp xgen-wmh $(@F)) \
	&& rm -f xgen-wmh xgen-wmh~

picmanwidgetsmarshal.c: picmanwidgetsmarshal.h
	$(AM_V_GEN) echo "#include \"picmanwidgetsmarshal.h\"" >> xgen-wmc \
	&& $(GLIB_GENMARSHAL) --prefix=_picman_widgets_marshal $(srcdir)/picmanwidgetsmarshal.list --body >> xgen-wmc \
	&& cp xgen-wmc $(@F) \
	&& rm -f xgen-wmc xgen-wmc~


picman-wilber-pixbufs.h: $(WILBER_IMAGES) Makefile.am
	$(AM_V_GEN) $(GDK_PIXBUF_CSOURCE) --raw --build-list $(WILBER_VARIABLES) > $(@F)


#
# test programs, not installed
#

EXTRA_PROGRAMS = \
	test-preview-area	\
	test-eevl


test_preview_area_SOURCES = test-preview-area.c 

test_preview_area_DEPENDENCIES = \
	$(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la	\
	$(top_builddir)/libpicmanwidgets/libpicmanwidgets-$(PICMAN_API_VERSION).la

test_preview_area_LDADD = \
	$(GTK_LIBS) \
	$(test_preview_area_DEPENDENCIES)


test_eevl_SOURCES = \
	test-eevl.c

test_eevl_DEPENDENCIES = \
	$(top_builddir)/libpicmanwidgets/libpicmanwidgets-$(PICMAN_API_VERSION).la

test_eevl_LDADD = \
	$(GLIB_LIBS) 		\
	$(test_eevl_DEPENDENCIES)


#
# test programs, not to be built by default and never installed
#

TESTS = test-eevl$(EXEEXT)




CLEANFILES += $(EXTRA_PROGRAMS)


install-data-local: install-ms-lib install-libtool-import-lib

uninstall-local: uninstall-ms-lib uninstall-libtool-import-lib
