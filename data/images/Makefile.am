## Makefile.am for picman/data/images

imagedatadir = $(picmandatadir)/images

if PICMAN_UNSTABLE
PICMAN_LOGO = picman-devel-logo.png
else
PICMAN_LOGO = picman-logo.png
endif

imagedata_DATA = \
	$(PICMAN_LOGO)	\
	picman-splash.png	\
	wilber.png

EXTRA_DIST = $(imagedata_DATA)
