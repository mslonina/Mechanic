ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src

install-exec-hook:
	cp mpimaps.sh $(DESTDIR)$(exec_prefix)
	$(LN_S) $(DESTDIR)$(bindir)/mpifarm $(DESTDIR)$(exec_prefix)/mpifarm
	$(LN_S) $(DESTDIR)$(docdir)/config  $(DESTDIR)$(exec_prefix)/config

clean-local:
	rm -rf $(DESTDIR)$(exec_prefix)

dist_doc_DATA = config
