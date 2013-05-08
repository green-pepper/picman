## Makefile.am for picman/desktop

desktop_in_files = picman.desktop.in.in
desktop_files = $(desktop_in_files:.desktop.in.in=.desktop)


if DESKTOP_DATADIR

applicationsdir = $(DESKTOP_DATADIR)/applications
applications_DATA = $(desktop_files)


icons16dir = $(DESKTOP_DATADIR)/icons/hicolor/16x16/apps
icons16_DATA = 16x16/picman.png

icons22dir = $(DESKTOP_DATADIR)/icons/hicolor/22x22/apps
icons22_DATA = 22x22/picman.png

icons24dir = $(DESKTOP_DATADIR)/icons/hicolor/24x24/apps
icons24_DATA = 24x24/picman.png

icons32dir = $(DESKTOP_DATADIR)/icons/hicolor/32x32/apps
icons32_DATA = 32x32/picman.png

icons48dir = $(DESKTOP_DATADIR)/icons/hicolor/48x48/apps
icons48_DATA = 48x48/picman.png

icons256dir = $(DESKTOP_DATADIR)/icons/hicolor/256x256/apps
icons256_DATA = 256x256/picman.png


# touch the toplevel icon directory as required by the icon theme spec
install-data-hook:
	touch $(DESTDIR)$(DESKTOP_DATADIR)/icons/hicolor

endif


EXTRA_DIST = \
	picman.desktop.in.in	\
	16x16/picman.png		\
	22x22/picman.png		\
	24x24/picman.png		\
	32x32/picman.png		\
	48x48/picman.png		\
	64x64/picman.png		\
	256x256/picman.png	


DISTCLEANFILES = $(desktop_files)


picman.desktop: picman.desktop.in $(wildcard $(top_srcdir)/po/*.po)
	$(INTLTOOL_MERGE) $(top_srcdir)/po $< $(@) -d -u -c $(top_builddir)/po/.intltool-merge-cache

validate: picman.desktop
	( dfvalidate=`which desktop-file-validate`; \
	  if test x$$dfvalidate != x && test -x $$dfvalidate; then \
            $$dfvalidate $< || ( echo "* $< INVALID *"; exit 1 ) \
	  else \
	    echo "Can't find desktop-file-validate to validate $<; proceed with fingers crossed..."; \
          fi )

dist-hook: validate
