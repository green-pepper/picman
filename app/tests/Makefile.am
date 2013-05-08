SUBDIRS = \
	files		\
	picmandir		\
	picmandir-empty

# Don't mess with user's picmandir. Pass in the abs top srcdir to the
# tests through an environment variable so they can set the picmandir
# they want to use
TESTS_ENVIRONMENT = \
	PICMAN_TESTING_ABS_TOP_SRCDIR=@abs_top_srcdir@ \
	PICMAN_TESTING_ABS_TOP_BUILDDIR=@abs_top_builddir@ \
	PICMAN_TESTING_PLUGINDIRS=@abs_top_builddir@/plug-ins/common \
	PICMAN_TESTING_PLUGINDIRS_BASENAME_IGNORES=mkgen.pl

# Run tests with xvfb-run if available
if HAVE_XVFB_RUN
TESTS_ENVIRONMENT += $(XVFB_RUN) --server-args="-screen 0 1280x1024x24"
endif


TESTS = \
	test-core					\
	test-picmanidtable				\
	test-save-and-export				\
	test-session-2-6-compatibility			\
	test-session-2-8-compatibility-multi-window	\
	test-session-2-8-compatibility-single-window	\
	test-single-window-mode				\
	test-tools					\
	test-ui						\
	test-xcf

EXTRA_PROGRAMS = $(TESTS)
CLEANFILES = $(EXTRA_PROGRAMS)

$(TESTS): picmandir-output

noinst_LIBRARIES = libpicmanapptestutils.a
libpicmanapptestutils_a_SOURCES = \
	picman-app-test-utils.c		\
	picman-app-test-utils.h		\
	picman-test-session-utils.c	\
	picman-test-session-utils.h

libpicmanbase = $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la
libpicmanconfig = $(top_builddir)/libpicmanconfig/libpicmanconfig-$(PICMAN_API_VERSION).la
libpicmancolor = $(top_builddir)/libpicmancolor/libpicmancolor-$(PICMAN_API_VERSION).la
libpicmanmath = $(top_builddir)/libpicmanmath/libpicmanmath-$(PICMAN_API_VERSION).la
libpicmanmodule = $(top_builddir)/libpicmanmodule/libpicmanmodule-$(PICMAN_API_VERSION).la
libpicmanwidgets = $(top_builddir)/libpicmanwidgets/libpicmanwidgets-$(PICMAN_API_VERSION).la
libpicmanthumb = $(top_builddir)/libpicmanthumb/libpicmanthumb-$(PICMAN_API_VERSION).la

AM_CPPFLAGS = \
	-I$(top_srcdir)		\
	-I$(top_srcdir)/app	\
	$(PANGOCAIRO_CFLAGS)	\
	$(GTK_CFLAGS)		\
	$(DBUS_GLIB_CFLAGS)	\
	$(GEGL_CFLAGS)		\
	-I$(includedir)

# We need this due to circular dependencies, see more detailed
# comments about it in app/Makefile.am
AM_LDFLAGS = \
	-Wl,-u,$(SYMPREFIX)xcf_init				\
	-Wl,-u,$(SYMPREFIX)internal_procs_init			\
	-Wl,-u,$(SYMPREFIX)picman_plug_in_manager_restore		\
	-Wl,-u,$(SYMPREFIX)picman_pdb_compat_param_spec		\
	-Wl,-u,$(SYMPREFIX)gui_init				\
	-Wl,-u,$(SYMPREFIX)plug_in_icc_profile_apply_rgb	\
	-Wl,-u,$(SYMPREFIX)picman_image_map_config_get_type	\
	-Wl,-u,$(SYMPREFIX)picman_vectors_undo_get_type		\
	-Wl,-u,$(SYMPREFIX)picman_vectors_mod_undo_get_type	\
	-Wl,-u,$(SYMPREFIX)picman_vectors_prop_undo_get_type	\
	-Wl,-u,$(SYMPREFIX)actions_init				\
	-Wl,-u,$(SYMPREFIX)picman_error_dialog_new		\
	-Wl,-u,$(SYMPREFIX)menus_save				\
	-Wl,-u,$(SYMPREFIX)picman_tools_save			\
	-Wl,-u,$(SYMPREFIX)picman_curve_map_pixels		\
	-Wl,-u,$(SYMPREFIX)picman_image_base_type			\
	-Wl,-u,$(SYMPREFIX)picman_param_spec_duplicate		\
	-Wl,-u,$(SYMPREFIX)picman_lebl_dialog

# Note that we have some duplicate entries here too to work around
# circular dependencies and systems on the same architectural layer as
# an alternative to LDFLAGS above
LDADD = \
	$(top_builddir)/app/dialogs/preferences-dialog.o	\
	$(top_builddir)/app/gui/libappgui.a			\
	$(top_builddir)/app/tools/libapptools.a			\
	$(top_builddir)/app/dialogs/libappdialogs.a		\
	$(top_builddir)/app/menus/libappmenus.a			\
	$(top_builddir)/app/actions/libappactions.a		\
	$(top_builddir)/app/dialogs/libappdialogs.a		\
	$(top_builddir)/app/display/libappdisplay.a		\
	$(top_builddir)/app/widgets/libappwidgets.a		\
	$(top_builddir)/app/xcf/libappxcf.a			\
	$(top_builddir)/app/pdb/libappinternal-procs.a		\
	$(top_builddir)/app/pdb/libapppdb.a			\
	$(top_builddir)/app/plug-in/libappplug-in.a		\
	$(top_builddir)/app/vectors/libappvectors.a		\
	$(top_builddir)/app/core/libappcore.a			\
	$(top_builddir)/app/file/libappfile.a			\
	$(top_builddir)/app/text/libapptext.a			\
	$(top_builddir)/app/paint/libapppaint.a			\
	$(top_builddir)/app/config/libappconfig.a		\
	$(top_builddir)/app/libapp.a				\
	$(top_builddir)/app/gegl/libappgegl.a			\
	$(top_builddir)/app/operations/libappoperations.a	\
	libpicmanapptestutils.a					\
	$(libpicmanwidgets)					\
	$(libpicmanconfig)					\
	$(libpicmanmath)						\
	$(libpicmanthumb)						\
	$(libpicmancolor)						\
	$(libpicmanmodule)					\
	$(libpicmanbase)						\
	$(PICMANICONRC)						\
	$(GTK_LIBS)						\
	$(DBUS_GLIB_LIBS)					\
	$(GDK_PIXBUF_LIBS)					\
	$(FREETYPE_LIBS)					\
	$(FONTCONFIG_LIBS)					\
	$(PANGOCAIRO_LIBS)					\
	$(CAIRO_LIBS)						\
	$(GEGL_LIBS)						\
	$(GLIB_LIBS)						\
	$(INTLLIBS)						\
	$(RT_LIBS)

picmandir-output:
	mkdir -p picmandir-output
	mkdir -p picmandir-output/brushes
	mkdir -p picmandir-output/patterns
	mkdir -p picmandir-output/gradients

clean-local:
	rm -rf picmandir-output
