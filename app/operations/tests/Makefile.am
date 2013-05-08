#TESTS = test-operations

EXTRA_PROGRAMS = $(TESTS)
CLEANFILES = $(EXTRA_PROGRAMS)

$(TESTS): output-dir

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
	$(GEGL_CFLAGS)		\
	-I$(includedir)

# We need this due to circular dependencies, see more detailed
# comments about it in app/Makefile.am
AM_LDFLAGS = \
	-Wl,-u,$(SYMPREFIX)xcf_init				\
	-Wl,-u,$(SYMPREFIX)internal_procs_init			\
	-Wl,-u,$(SYMPREFIX)picman_plug_in_manager_restore		\
	-Wl,-u,$(SYMPREFIX)picman_pdb_compat_param_spec		\
	-Wl,-u,$(SYMPREFIX)plug_in_icc_profile_apply_rgb	\
	-Wl,-u,$(SYMPREFIX)picman_vectors_undo_get_type		\
	-Wl,-u,$(SYMPREFIX)picman_vectors_mod_undo_get_type	\
	-Wl,-u,$(SYMPREFIX)picman_vectors_prop_undo_get_type	

# Note that we have some duplicate entries here too to work around
# circular dependencies and systems on the same architectural layer as
# an alternative to LDFLAGS above
LDADD = \
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
	$(libpicmanconfig)					\
	$(libpicmanmath)						\
	$(libpicmanthumb)						\
	$(libpicmancolor)						\
	$(libpicmanmodule)					\
	$(libpicmanbase)						\
	$(GDK_PIXBUF_LIBS)					\
	$(PANGOCAIRO_LIBS)					\
	$(GEGL_LIBS)						\
	$(GLIB_LIBS)						

output-dir:
	mkdir -p output

clean-local:
	rm -rf output
