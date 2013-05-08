
noinst_DATA = pypicman.html

EXTRA_DIST = \
	pypicman.sgml			\
	pypicman.html			\
	structure-of-plugin.html	\
	procedural-database.html	\
	picman-module-procedures.html	\
	picman-objects.html		\
	support-modules.html		\
	end-note.html

printed: pypicman.ps pypicman.pdf

#clean:
#	rm -f *.html pypicman.ps pypicman.pdf


pypicman.ps: pypicman.sgml
	db2ps $(srcdir)/pypicman.sgml

pypicman.pdf: pypicman.sgml
	db2pdf $(srcdir)/pypicman.sgml

pypicman.html: pypicman.sgml
	cd $(srcdir) && db2html --nochunks pypicman.sgml
	cd $(srcdir) && mv pypicman/*.html .
	rm -rf $(srcdir)/pypicman

.PHONY: printed #clean

