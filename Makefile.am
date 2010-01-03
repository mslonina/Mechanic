ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src
EXTRA_DIST = doc/mpifarm-config doc/qfarm.sh doc/hello.tgz
dist_doc_DATA = $(EXTRA_DIST)

clean-local:
	rm -rf $(DESTDIR)$(exec_prefix)

uninstall-local:
	rm -rf $(DESTDIR)$(docdir)


