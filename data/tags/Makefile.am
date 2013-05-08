## Makefile.am for picman/data/tags

tagsdatadir = $(picmandatadir)/tags

tagsdata_in_files = picman-tags-default.xml.in

tagsdata_data_files = $(tagsdata_in_files:.xml.in=.xml)

tagsdata_DATA =	$(tagsdata_data_files)

EXTRA_DIST = \
	$(tagsdata_DATA)	\
	$(tagsdata_in_files)	\
	picman-tags.dtd		

DISTCLEANFILES = $(tagsdata_data_files)


picman-tags-default.xml: picman-tags-default.xml.in $(wildcard $(top_srcdir)/po-tags/*.po)
	$(INTLTOOL_MERGE) $(top_srcdir)/po-tags $< $(@) -x -u -c $(top_builddir)/po-tags/.intltool-merge-cache


validate: picman-tags-default.xml
if HAVE_XMLLINT
	@$(XMLLINT) --noout --path $(srcdir) --valid $< || \
	  ( echo "* picman-tags-default.xml INVALID *"; exit 1; )
endif

dist-hook: validate
