## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Picman-PDB\"

INCLUDES = \
	-I$(top_builddir)	\
	-I$(top_srcdir)		\
	-I$(top_builddir)/app	\
	-I$(top_srcdir)/app	\
	$(CAIRO_CFLAGS)		\
	$(GEGL_CFLAGS)		\
	$(GDK_PIXBUF_CFLAGS)	\
	-I$(includedir)

noinst_LIBRARIES = libapppdb.a libappinternal-procs.a

libapppdb_a_SOURCES = \
	pdb-types.h			\
	\
	picman-pdb-compat.c		\
	picman-pdb-compat.h		\
	picmanpdb.c			\
	picmanpdb.h			\
	picmanpdb-query.c			\
	picmanpdb-query.h			\
	picmanpdb-utils.c			\
	picmanpdb-utils.h			\
	picmanpdbcontext.c		\
	picmanpdbcontext.h		\
	picmanpdberror.c			\
	picmanpdberror.h			\
	picmanprocedure.c			\
	picmanprocedure.h

libappinternal_procs_a_SOURCES = \
	internal-procs.c		\
	internal-procs.h		\
	brush-cmds.c			\
	brush-select-cmds.c		\
	brushes-cmds.c			\
	buffer-cmds.c			\
	channel-cmds.c			\
	color-cmds.c			\
	context-cmds.c			\
	convert-cmds.c			\
	display-cmds.c			\
	drawable-cmds.c			\
	drawable-transform-cmds.c	\
	dynamics-cmds.c			\
	edit-cmds.c			\
	fileops-cmds.c			\
	floating-sel-cmds.c		\
	font-select-cmds.c		\
	fonts-cmds.c			\
	picman-cmds.c			\
	picmanrc-cmds.c			\
	gradient-cmds.c			\
	gradient-select-cmds.c		\
	gradients-cmds.c		\
	grid-cmds.c			\
	guides-cmds.c			\
	help-cmds.c			\
	image-cmds.c			\
	image-select-cmds.c		\
	item-cmds.c			\
	item-transform-cmds.c		\
	layer-cmds.c			\
	message-cmds.c			\
	paint-tools-cmds.c		\
	palette-cmds.c			\
	palette-select-cmds.c		\
	palettes-cmds.c			\
	paths-cmds.c			\
	pattern-cmds.c			\
	pattern-select-cmds.c		\
	patterns-cmds.c			\
	plug-in-cmds.c			\
	plug-in-compat-cmds.c		\
	procedural-db-cmds.c		\
	progress-cmds.c			\
	selection-cmds.c		\
	selection-tools-cmds.c		\
	text-layer-cmds.c		\
	text-tool-cmds.c		\
	transform-tools-cmds.c		\
	undo-cmds.c			\
	unit-cmds.c			\
	vectors-cmds.c
