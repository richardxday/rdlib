
AUTOMAKE_OPTIONS = foreign

include doxygen.am

SUBDIRS = src

EXTRA_DIST = README COPYING autogen.sh

pkgconfig_DATA = rdlib-$(RDLIB_MAJORMINOR).pc

rdlib-$(RDLIB_MAJORMINOR).pc: rdlib.pc
	cp rdlib.pc rdlib-$(RDLIB_MAJORMINOR).pc

rdlib-$(RDLIB_MAJORMINOR)-uninstalled.pc: rdlib-uninstalled.pc
	cp rdlib-uninstalled.pc rdlib-$(RDLIB_MAJORMINOR)-uninstalled.pc

BUILT_SOURCES = rdlib-$(RDLIB_MAJORMINOR)-uninstalled.pc

CLEANFILES = rdlib-$(RDLIB_MAJORMINOR).pc rdlib-$(RDLIB_MAJORMINOR)-uninstalled.pc

ACLOCAL_FLAGS = -I m4
