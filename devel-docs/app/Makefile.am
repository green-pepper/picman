## Process this file with automake to produce Makefile.in

# The name of the module.
DOC_MODULE = app

# The top-level SGML file.
DOC_MAIN_SGML_FILE = $(DOC_MODULE)-docs.sgml

# The directory containing the source code.
DOC_SOURCE_DIR = $(top_srcdir)/$(DOC_MODULE)

# Extra options to pass to gtkdoc-scangobj
SCANGOBJ_OPTIONS =

# Extra options to supply to gtkdoc-scan
SCAN_OPTIONS =

# Extra options to supply to gtkdoc-mkdb
MKDB_OPTIONS = --sgml-mode --output-format=xml

# Extra options to supply to gtkdoc-fixref
FIXXREF_OPTIONS = --extra-dir=../libpicmanbase/html --extra-dir=../libpicmancolor/html --extra-dir=../libpicmanconfig/html --extra-dir=../libpicmanmath/html --extra-dir=../libpicmanwidgets/html


# Used for dependencies
HFILE_GLOB = $(DOC_SOURCE_DIR)/*.h
CFILE_GLOB = $(DOC_SOURCE_DIR)/*.c

# Header files to ignore when scanning
IGNORE_HFILES = \
	picman-intl.h			\
	picmandbusservice-glue.h		\
	picmanimage-convert-data.h	\
	picmanimage-convert-fsdither.h	\
	picmanhelp-ids.h			\
	picmanrc-blurbs.h

# Images to copy into HTML directory
HTML_IMAGES =

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE)
content_files =

# CFLAGS and LDFLAGS for compiling scan program. Only needed
# if $(DOC_MODULE).types is non-empty.

GTKDOC_CFLAGS = \
	-g		\
	-I$(top_srcdir)	\
	$(GTK_CFLAGS)	\
	-I$(includedir)	\
	-UGTK_DISABLE_SINGLE_INCLUDES

GTKDOC_LIBS = \
	-Wl,-u,$(SYMPREFIX)xcf_init		   			     \
	-Wl,-u,$(SYMPREFIX)internal_procs_init 				     \
	-Wl,-u,$(SYMPREFIX)picman_coords_mix	   			     \
	-Wl,-u,$(SYMPREFIX)picman_plug_in_manager_restore			     \
	-Wl,-u,$(SYMPREFIX)picman_lebl_dialog				     \
	-Wl,-u,$(SYMPREFIX)picman_param_spec_duplicate			     \
	$(top_builddir)/app/app.o					     \
        $(top_builddir)/app/batch.o					     \
        $(top_builddir)/app/errors.o					     \
        $(top_builddir)/app/language.o					     \
        $(top_builddir)/app/sanity.o					     \
        $(top_builddir)/app/units.o					     \
	$(top_builddir)/app/picman-debug.o				     \
	$(top_builddir)/app/picman-log.o					     \
        $(top_builddir)/app/gui/libappgui.a				     \
	$(top_builddir)/app/actions/libappactions.a			     \
        $(top_builddir)/app/dialogs/libappdialogs.a			     \
        $(top_builddir)/app/menus/libappmenus.a				     \
        $(top_builddir)/app/tools/libapptools.a				     \
        $(top_builddir)/app/display/libappdisplay.a			     \
        $(top_builddir)/app/widgets/libappwidgets.a			     \
	$(top_builddir)/app/pdb/libappinternal-procs.a			     \
        $(top_builddir)/app/xcf/libappxcf.a				     \
        $(top_builddir)/app/file/libappfile.a				     \
        $(top_builddir)/app/plug-in/libappplug-in.a			     \
        $(top_builddir)/app/pdb/libapppdb.a				     \
        $(top_builddir)/app/core/libappcore.a				     \
        $(top_builddir)/app/vectors/libappvectors.a			     \
        $(top_builddir)/app/paint/libapppaint.a				     \
        $(top_builddir)/app/text/libapptext.a				     \
        $(top_builddir)/app/config/libappconfig.a			     \
        $(top_builddir)/app/gegl/libappgegl.a				     \
        $(top_builddir)/app/operations/libappoperations.a		     \
        $(top_builddir)/libpicmanwidgets/libpicmanwidgets-$(PICMAN_API_VERSION).la \
        $(top_builddir)/libpicmanmodule/libpicmanmodule-$(PICMAN_API_VERSION).la   \
        $(top_builddir)/libpicmancolor/libpicmancolor-$(PICMAN_API_VERSION).la     \
        $(top_builddir)/libpicmanthumb/libpicmanthumb-$(PICMAN_API_VERSION).la     \
        $(top_builddir)/libpicmanmath/libpicmanmath-$(PICMAN_API_VERSION).la       \
        $(top_builddir)/libpicmanconfig/libpicmanconfig-$(PICMAN_API_VERSION).la   \
        $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la       \
        $(GTK_LIBS)                                                          \
        $(LIBART_LIBS)                                                       \
        $(PANGOFT2_LIBS)                                                     \
        $(FONTCONFIG_LIBS)                                                   \
        $(FREETYPE_LIBS)                                                     \
	$(DBUS_GLIB_LIBS)						     \
	$(GEGL_LIBS)							     \
	$(RT_LIBS)							     \
        $(INTLLIBS)


if ENABLE_GTK_DOC_APP

include $(top_srcdir)/gtk-doc.make

else

EXTRA_DIST = \
	$(DOC_MODULE)-docs.sgml		\
	$(DOC_MODULE)-overrides.txt	\
	$(DOC_MODULE)-sections.txt

endif

# Other files to distribute
EXTRA_DIST += \
	$(DOC_MODULE).types	\
	version.in
