## Process this file with automake to produce Makefile.in

libpicmanbase = $(top_builddir)/libpicmanbase/libpicmanbase-$(PICMAN_API_VERSION).la
libpicmanconfig = $(top_builddir)/libpicmanconfig/libpicmanconfig-$(PICMAN_API_VERSION).la
libpicmancolor = $(top_builddir)/libpicmancolor/libpicmancolor-$(PICMAN_API_VERSION).la
libpicmanmath = $(top_builddir)/libpicmanmath/libpicmanmath-$(PICMAN_API_VERSION).la
libpicmanmodule = $(top_builddir)/libpicmanmodule/libpicmanmodule-$(PICMAN_API_VERSION).la
libpicmanwidgets = $(top_builddir)/libpicmanwidgets/libpicmanwidgets-$(PICMAN_API_VERSION).la
libpicmanthumb = $(top_builddir)/libpicmanthumb/libpicmanthumb-$(PICMAN_API_VERSION).la

# Sort this by architectural dependencies, lowest level at the top,
# so that when e.g. changing a header-file the subdirs are built in
# the right order
SUBDIRS = \
	config		\
	core		\
	operations	\
	gegl		\
	text		\
	vectors		\
	paint		\
	plug-in		\
	xcf		\
	file		\
	pdb		\
	widgets		\
	display		\
	tools		\
	dialogs		\
	actions		\
	menus		\
	gui		\
	.		\
	tests

# Put the PICMAN core in a lib so we can conveniently link against that
# in test cases
noinst_LIBRARIES = libapp.a

if ENABLE_PICMAN_CONSOLE
bin_PROGRAMS = picman-@PICMAN_APP_VERSION@ picman-console-@PICMAN_APP_VERSION@
else
bin_PROGRAMS = picman-@PICMAN_APP_VERSION@
endif

libapp_sources = \
	about.h		\
	app.c		\
	app.h		\
	batch.c		\
	batch.h		\
	errors.c	\
	errors.h	\
	language.c	\
	language.h	\
	sanity.c	\
	sanity.h	\
	signals.c	\
	signals.h	\
	tests.c		\
	tests.h		\
	unique.c	\
	unique.h	\
	units.c		\
	units.h		\
	version.c	\
	version.h	\
	picman-debug.c	\
	picman-debug.h	\
	picman-log.c	\
	picman-log.h	\
	picman-intl.h

libapp_generated_sources = \
	git-version.h

# Build git-version.h before anything in the subdirs as this is needed
# in the about dialog.
BUILT_SOURCES = $(libapp_generated_sources)

CLEANFILES = $(libapp_generated_sources)

$(srcdir)/version.c: git-version.h

libapp_a_SOURCES = $(libapp_sources) $(libapp_generated_sources)

picman_@PICMAN_APP_VERSION@_SOURCES = $(libapp_sources) main.c


if PLATFORM_OSX
framework_carbon = -framework Carbon
endif

if OS_WIN32
win32_ldflags = -mwindows -Wl,--tsaware $(WIN32_LARGE_ADDRESS_AWARE)
endif

if USE_BINRELOC
munix = -Wl,-rpath '-Wl,$$ORIGIN/../lib'
endif

if HAVE_WINDRES
include $(top_srcdir)/build/windows/picmanrc.rule
PICMANRC = picman-$(PICMAN_APP_VERSION).rc.o
PICMANCONSOLERC = picman-console-$(PICMAN_APP_VERSION).rc.o
endif

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Picman\"		\
	-DPICMAN_APP_GLUE_COMPILATION

INCLUDES = \
	-I$(top_srcdir)		\
	$(GTK_CFLAGS)		\
	$(PANGOCAIRO_CFLAGS)	\
	$(GEGL_CFLAGS)		\
	-I$(includedir)

# FIXME: core should not depend on xcf
workaround_that_core_depends_on_xcf = \
	-Wl,-u,$(SYMPREFIX)xcf_init

# FIXME: core should not depend on pdb
workaround_that_core_depends_on_pdb = \
	-Wl,-u,$(SYMPREFIX)internal_procs_init		\
	-Wl,-u,$(SYMPREFIX)picman_plug_in_manager_restore

# FIXME: plug-in should not depend on pdb
workaround_that_plug_in_depends_on_pdb = \
	-Wl,-u,$(SYMPREFIX)picman_pdb_compat_param_spec

# FIXME: file should not depend on plug-in
workaround_that_file_depends_on_plug_in = \
	-Wl,-u,$(SYMPREFIX)plug_in_icc_profile_apply_rgb	\
	-Wl,-u,$(SYMPREFIX)picman_image_map_config_get_type

# core, vectors and gegl are on the same architectural layer, prevent
# the linker from panicing
calm_down_linker = \
	-Wl,-u,$(SYMPREFIX)picman_vectors_undo_get_type		\
	-Wl,-u,$(SYMPREFIX)picman_vectors_mod_undo_get_type	\
	-Wl,-u,$(SYMPREFIX)picman_vectors_prop_undo_get_type	\
	-Wl,-u,$(SYMPREFIX)picman_curve_map_pixels		\
	-Wl,-u,$(SYMPREFIX)picman_param_spec_duplicate		\
	-Wl,-u,$(SYMPREFIX)picman_operations_init

AM_LDFLAGS = \
	$(munix)					\
	$(calm_down_linker)				\
	$(workaround_that_core_depends_on_xcf)		\
	$(workaround_that_core_depends_on_pdb)		\
	$(workaround_that_plug_in_depends_on_pdb)	\
	$(workaround_that_file_depends_on_plug_in)

picmanconsoleldadd = \
	xcf/libappxcf.a			\
	pdb/libappinternal-procs.a	\
	pdb/libapppdb.a			\
	plug-in/libappplug-in.a		\
	vectors/libappvectors.a		\
	core/libappcore.a		\
	file/libappfile.a		\
	text/libapptext.a		\
	paint/libapppaint.a		\
	operations/libappoperations.a	\
	gegl/libappgegl.a		\
	config/libappconfig.a		\
	$(libpicmanconfig)		\
	$(libpicmanmath)			\
	$(libpicmanthumb)			\
	$(libpicmancolor)			\
	$(libpicmanmodule)		\
	$(libpicmanbase)			\
	$(GDK_PIXBUF_LIBS)		\
	$(FREETYPE_LIBS)		\
	$(FONTCONFIG_LIBS)		\
	$(PANGOCAIRO_LIBS)		\
	$(CAIRO_LIBS)			\
	$(GEGL_LIBS)			\
	$(GLIB_LIBS)			\
	$(INTLLIBS)			\
	$(RT_LIBS)

picman_@PICMAN_APP_VERSION@_LDFLAGS = \
	$(AM_LDFLAGS)		\
	$(win32_ldflags)	\
	$(framework_carbon)	\
	-Wl,-u,$(SYMPREFIX)picman_lebl_dialog

picman_@PICMAN_APP_VERSION@_LDADD = \
	gui/libappgui.a		\
	menus/libappmenus.a	\
	actions/libappactions.a	\
	dialogs/libappdialogs.a	\
	tools/libapptools.a	\
	display/libappdisplay.a	\
	widgets/libappwidgets.a	\
	$(libpicmanwidgets)	\
	$(GTK_LIBS)		\
	$(DBUS_GLIB_LIBS)	\
	$(picmanconsoleldadd)	\
	$(PICMANRC)


if ENABLE_PICMAN_CONSOLE

picman_console_@PICMAN_APP_VERSION@_SOURCES = $(libapp_sources) main.c

picman_console_@PICMAN_APP_VERSION@_CPPFLAGS = \
	$(AM_CPPFLAGS)			\
	-DPICMAN_CONSOLE_COMPILATION

picman_console_@PICMAN_APP_VERSION@_LDADD = \
	$(picmanconsoleldadd) \
	$(PICMANCONSOLERC)

endif


install-exec-hook:
if DEFAULT_BINARY
	cd $(DESTDIR)$(bindir) \
	&& rm -f picman$(EXEEXT) \
	&& $(LN_S) picman-$(PICMAN_APP_VERSION)$(EXEEXT) picman$(EXEEXT)
if ENABLE_PICMAN_CONSOLE
	cd $(DESTDIR)$(bindir) \
	&& rm -f picman-console$(EXEEXT) \
	&& $(LN_S) picman-console-$(PICMAN_APP_VERSION)$(EXEEXT) picman-console$(EXEEXT)
endif
endif

uninstall-local:
if DEFAULT_BINARY
	rm -f $(DESTDIR)$(bindir)/picman$(EXEEXT)
if ENABLE_PICMAN_CONSOLE
	rm -f $(DESTDIR)$(bindir)/picman-console$(EXEEXT)
endif
endif


# require picman-console when making dist
#
if ENABLE_PICMAN_CONSOLE
dist-check-picman-console:
else
dist-check-picman-console:
	@echo "*** picman-console must be enabled in order to make dist"
	@false
endif


# hook to assure that the system picmanrc and the picmanrc manpage are
# uptodate when a release is made
#
dist-dump-picmanrc: picman-console-$(PICMAN_APP_VERSION)$(EXEEXT)
	./$< --dump-picmanrc-system > picmanrc.tmp	\
		&& sed -e "s/num-processors [0-9]*/num-processors 1/"	\
		   picmanrc.tmp > picmanrc.tmp2				\
		&& (cmp -s picmanrc.tmp2 $(top_srcdir)/etc/picmanrc ||	\
		    cp picmanrc.tmp2 $(top_srcdir)/etc/picmanrc)		\
		&& rm picmanrc.tmp picmanrc.tmp2
	./$< --dump-picmanrc-manpage > picmanrc.tmp	\
		&& sed -e "s/num-processors [0-9]*/num-processors 1/"	\
		   picmanrc.tmp > picmanrc.tmp2				\
		&& (cmp -s picmanrc.tmp2 $(top_srcdir)/docs/picmanrc.5.in ||\
		    cp picmanrc.tmp2 $(top_srcdir)/docs/picmanrc.5.in)	\
		&& rm picmanrc.tmp picmanrc.tmp2

dist-hook: dist-check-picman-console dist-dump-picmanrc

# If git is available, always check if git-version.h should be
# updated. If git is not available, don't do anything if git-version.h
# already exists because then we are probably working with a tarball
# in which case the git-version.h we ship is correct.
git-version.h: update-git-version-header
	@if test -d "$(top_srcdir)/.git"; then \
	  git_version="`git --git-dir=$(top_srcdir)/.git describe --always`"; \
	  git_last_commit_year="`git --git-dir=$(top_srcdir)/.git log -n1 --reverse --pretty=%ci | cut -b 1-4`"; \
	elif test ! -f "$@"; then \
	  git_version="Unknown, shouldn't happen"; \
	  git_last_commit_timestamp=-1; \
	  git_last_commit_year="`date -u '+%Y'`"; \
	fi; \
	if test -n "$$git_version"; then \
	  echo "#ifndef __GIT_VERSION_H__" > "$@.tmp"; \
	  echo "#define __GIT_VERSION_H__" >> "$@.tmp"; \
	  echo "#define PICMAN_GIT_VERSION \"$$git_version\"" >> "$@.tmp"; \
	  echo "#define PICMAN_GIT_LAST_COMMIT_YEAR \"$$git_last_commit_year\"" >> "$@.tmp"; \
	  echo "#endif /* __GIT_VERSION_H__ */" >> "$@.tmp"; \
	fi
	@if ( test -f "$@.tmp" && test -f "$@" && cmp "$@.tmp" "$@" > /dev/null ); then \
	  rm -f "$@.tmp"; \
	elif test -f "$@.tmp"; then \
	  mv "$@.tmp" "$@"; \
	  echo "  git HEAD changed: $@ regenerated"; \
	fi

.PHONY: update-git-version-header
