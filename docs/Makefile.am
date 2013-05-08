## Process this file with automake to produce Makefile.in

EXTRA_DIST = \
	Wilber.xcf.gz			\
	Wilber.xcf.gz.README		\
	Wilber_Construction_Kit.xcf.gz

mans = \
	picman-$(PICMAN_APP_VERSION).1	\
	picmanrc-$(PICMAN_APP_VERSION).5

man_MANS = \
	$(mans)				\
	picmantool-$(PICMAN_TOOL_VERSION).1

default_binary_mans = $(mans)
if ENABLE_PICMAN_CONSOLE
default_binary_mans += picman-console-$(PICMAN_APP_VERSION).1
endif

install-data-hook:
if ENABLE_PICMAN_CONSOLE
	cd $(DESTDIR)$(mandir)/man1; \
	rm -f picman-console-$(PICMAN_APP_VERSION).1; \
	ln -s picman-$(PICMAN_APP_VERSION).1 picman-console-$(PICMAN_APP_VERSION).1
endif
if DEFAULT_BINARY
	@list='$(default_binary_mans)'; \
	for i in $$list; do \
	  s=`echo $$i | sed -e 's/.*\\(.\\)$$/\1/'`; \
	  n=`echo $$i | sed -e 's/-[^-]*$$//'`; \
	  f="$$n.$$s"; \
	  echo "cd $(DESTDIR)$(mandir)/man$$s"; \
	  cd $(DESTDIR)$(mandir)/man$$s; \
	  echo "rm -f $$f"; \
	  rm -f $$f; \
	  echo "ln -s $$i $$f"; \
	  ln -s $$i $$f; \
	done
endif

uninstall-local:
if ENABLE_PICMAN_CONSOLE
	rm -f $(DESTDIR)$(mandir)/man1/picman-console-$(PICMAN_APP_VERSION).1
endif
if DEFAULT_BINARY
	@list='$(default_binary_mans)'; \
	for i in $$list; do \
	  s=`echo $$i | sed -e 's/.*\\(.\\)$$/\1/'`; \
	  n=`echo $$i | sed -e 's/-[^-]*$$//'`; \
	  f="$$n.$$s"; \
	  echo " rm -f $(DESTDIR)$(mandir)/man$$s/$$f"; \
	  rm -f $(DESTDIR)$(mandir)/man$$s/$$f; \
	done
endif
