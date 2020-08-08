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
gsettings_SCHEMAS = net.nine-mens-morris.gschema.xml

EXTRA_DIST += $(gsettings_SCHEMAS) $(sys_in_files)
DISTCLEANFILES = $(sys_DATA)

@INTLTOOL_DESKTOP_RULE@
@GSETTINGS_RULES@
