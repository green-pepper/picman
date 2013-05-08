## Makefile.am for picman/data/tips

tipsdatadir = $(picmandatadir)/tips

tipsdata_in_files = picman-tips.xml.in

tipsdata_data_files = $(tipsdata_in_files:.xml.in=.xml)

tipsdata_DATA =	$(tipsdata_data_files)

EXTRA_DIST = \
	$(tipsdata_DATA)	\
	$(tipsdata_in_files)	\
        picman-tips.dtd		\
        fortunes.xsl

DISTCLEANFILES = $(tipsdata_data_files)


picman-tips.xml: picman-tips.xml.in $(wildcard $(top_srcdir)/po-tips/*.po)
	$(INTLTOOL_MERGE) $(top_srcdir)/po-tips $< $(@) -x -u -c $(top_builddir)/po-tips/.intltool-merge-cache


validate: picman-tips.xml
if HAVE_XMLLINT
	@$(XMLLINT) --noout --path $(srcdir) --valid $< || \
	  ( echo "* picman-tips.xml INVALID *"; exit 1; )
endif

fortunes: picman-tips.xml fortunes.xsl
if HAVE_XSLTPROC
	$(XSLTPROC) --stringparam lang en fortunes.xsl $< > $(@) || rm -f $(@)
else
	@echo "xsltproc is needed to build fortunes"; exit 1;
endif


dist-hook: validate
