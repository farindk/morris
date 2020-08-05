## Makefile.am for morris

SUBDIRS = src po pixmaps

EXTRA_DIST = build-aux/config.rpath m4/ChangeLog  build-aux/config.rpath \
  debian/changelog debian/compat debian/control debian/copyright debian/docs debian/rules


ACLOCAL_AMFLAGS = -I m4

#  morris.desktop.in.in morris.schemas.in

sysdir = $(datadir)/applications
sys_in_files = morris.desktop.in
sys_DATA = $(sys_in_files:.desktop.in=.desktop)

schema_in_files = morris.schemas.in
if HAVE_GCONF
  schemadir = $(GCONF_SCHEMA_FILE_DIR)
  schema_DATA = $(schema_in_files:.schemas.in=.schemas)
endif

EXTRA_DIST += $(schema_in_files) $(sys_in_files)
DISTCLEANFILES = $(sys_DATA) $(schema_DATA)

install-data-local:
if HAVE_GCONF
if GCONF_SCHEMAS_INSTALL
        GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/morris/$(schema_DATA)
endif
endif

@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_SCHEMAS_RULE@
