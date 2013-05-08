## Process this file with automake to produce Makefile.in
SUBDIRS = tests

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Picman-Operations\"

INCLUDES = \
	-I$(top_builddir)	\
	-I$(top_srcdir)		\
	-I$(top_builddir)/app	\
	-I$(top_srcdir)/app	\
	$(CAIRO_CFLAGS)		\
	$(GEGL_CFLAGS)		\
	$(GDK_PIXBUF_CFLAGS)	\
	-I$(includedir)

noinst_LIBRARIES = libappoperations.a

libappoperations_a_sources = \
	operations-types.h			\
	picman-operations.c			\
	picman-operations.h			\
	\
	picmanbrightnesscontrastconfig.c		\
	picmanbrightnesscontrastconfig.h		\
	picmancageconfig.c			\
	picmancageconfig.h			\
	picmancolorbalanceconfig.c		\
	picmancolorbalanceconfig.h		\
	picmancolorizeconfig.c			\
	picmancolorizeconfig.h			\
	picmancurvesconfig.c			\
	picmancurvesconfig.h			\
	picmandesaturateconfig.c			\
	picmandesaturateconfig.h			\
	picmanhuesaturationconfig.c		\
	picmanhuesaturationconfig.h		\
	picmanlevelsconfig.c			\
	picmanlevelsconfig.h			\
	picmanposterizeconfig.c			\
	picmanposterizeconfig.h			\
	picmanthresholdconfig.c			\
	picmanthresholdconfig.h			\
	\
	picmanoperationborder.c			\
	picmanoperationborder.h			\
	picmanoperationcagecoefcalc.c		\
	picmanoperationcagecoefcalc.h		\
	picmanoperationcagetransform.c		\
	picmanoperationcagetransform.h		\
	picmanoperationequalize.c			\
	picmanoperationequalize.h			\
	picmanoperationgrow.c			\
	picmanoperationgrow.h			\
	picmanoperationhistogramsink.c		\
	picmanoperationhistogramsink.h		\
	picmanoperationmaskcomponents.c		\
	picmanoperationmaskcomponents.h		\
	picmanoperationsemiflatten.c		\
	picmanoperationsemiflatten.h		\
	picmanoperationsetalpha.c			\
	picmanoperationsetalpha.h			\
	picmanoperationshapeburst.c		\
	picmanoperationshapeburst.h		\
	picmanoperationshrink.c			\
	picmanoperationshrink.h			\
	picmanoperationthresholdalpha.c		\
	picmanoperationthresholdalpha.h		\
	\
	picmanoperationpointfilter.c		\
	picmanoperationpointfilter.h		\
	picmanoperationbrightnesscontrast.c	\
	picmanoperationbrightnesscontrast.h	\
	picmanoperationcolorbalance.c		\
	picmanoperationcolorbalance.h		\
	picmanoperationcolorize.c			\
	picmanoperationcolorize.h			\
	picmanoperationcurves.c			\
	picmanoperationcurves.h			\
	picmanoperationdesaturate.c		\
	picmanoperationdesaturate.h		\
	picmanoperationhuesaturation.c		\
	picmanoperationhuesaturation.h		\
	picmanoperationlevels.c			\
	picmanoperationlevels.h			\
	picmanoperationposterize.c		\
	picmanoperationposterize.h		\
	picmanoperationthreshold.c		\
	picmanoperationthreshold.h		\
	\
	picmanoperationpointlayermode.c		\
	picmanoperationpointlayermode.h		\
	picmanoperationnormalmode.c		\
	picmanoperationnormalmode.h		\
	picmanoperationdissolvemode.c     	\
	picmanoperationdissolvemode.h     	\
	picmanoperationbehindmode.c       	\
	picmanoperationbehindmode.h       	\
	picmanoperationmultiplymode.c     	\
	picmanoperationmultiplymode.h     	\
	picmanoperationscreenmode.c       	\
	picmanoperationscreenmode.h       	\
	picmanoperationoverlaymode.c      	\
	picmanoperationoverlaymode.h      	\
	picmanoperationdifferencemode.c   	\
	picmanoperationdifferencemode.h   	\
	picmanoperationadditionmode.c     	\
	picmanoperationadditionmode.h     	\
	picmanoperationsubtractmode.c     	\
	picmanoperationsubtractmode.h     	\
	picmanoperationdarkenonlymode.c   	\
	picmanoperationdarkenonlymode.h   	\
	picmanoperationlightenonlymode.c  	\
	picmanoperationlightenonlymode.h  	\
	picmanoperationhuemode.c          	\
	picmanoperationhuemode.h          	\
	picmanoperationsaturationmode.c   	\
	picmanoperationsaturationmode.h   	\
	picmanoperationcolormode.c        	\
	picmanoperationcolormode.h        	\
	picmanoperationvaluemode.c        	\
	picmanoperationvaluemode.h        	\
	picmanoperationdividemode.c       	\
	picmanoperationdividemode.h       	\
	picmanoperationdodgemode.c        	\
	picmanoperationdodgemode.h        	\
	picmanoperationburnmode.c         	\
	picmanoperationburnmode.h         	\
	picmanoperationhardlightmode.c    	\
	picmanoperationhardlightmode.h    	\
	picmanoperationsoftlightmode.c    	\
	picmanoperationsoftlightmode.h    	\
	picmanoperationgrainextractmode.c 	\
	picmanoperationgrainextractmode.h 	\
	picmanoperationgrainmergemode.c   	\
	picmanoperationgrainmergemode.h   	\
	picmanoperationcolorerasemode.c   	\
	picmanoperationcolorerasemode.h   	\
	picmanoperationerasemode.c        	\
	picmanoperationerasemode.h        	\
	picmanoperationreplacemode.c      	\
	picmanoperationreplacemode.h      	\
	picmanoperationantierasemode.c    	\
	picmanoperationantierasemode.h

libappoperations_a_SOURCES = $(libappoperations_a_sources)
