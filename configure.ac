
AC_PREREQ([2.63])
AC_INIT([morris],[0.2],[dirk.farin@gmail.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([src/morris.cc])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([-Wall -Werror foreign dist-bzip2])

#AM_MAINTAINER_MODE
#AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INTLTOOL
AC_PROG_LIBTOOL

# Checks for libraries.
PKG_CHECK_MODULES(GTK, [
glib-2.0
gthread-2.0
gtk+-2.0 >= 2.4
])

AC_ARG_WITH([gconf],
  AS_HELP_STRING([--without-gconf], [Build without gconf library (default: test)]))

if test "x$with_gconf" != "xno"; then
  PKG_CHECK_MODULES(GCONF, [gconf-2.0])
  AC_DEFINE(HAVE_GCONF, 1, "compile with gconf support")
fi
AM_GCONF_SOURCE_2
AM_CONDITIONAL(HAVE_GCONF, test "x$with_gconf" != "xno")


# gdk-pixbuf-2.0
# libgnome-2.0 libgnomeui-2.0 

#ACLOCAL_AMFLAGS = -I build-aux

BOOST_REQUIRE
BOOST_SIGNALS2()


# Checks for header files.
AC_CHECK_HEADERS([libintl.h locale.h stdlib.h string.h sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AM_GNU_GETTEXT([external])

GETTEXT_PACKAGE=morris
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [The gettext package name])

AC_FUNC_MALLOC
AC_CHECK_FUNCS([dup2 gettimeofday memset])


# === target specific options ===

AC_MSG_CHECKING([for target architecture])
case x"$target" in
  xNONE | x)
    target_or_host="$host" ;;
  *)
    target_or_host="$target" ;;
esac
AC_MSG_RESULT([$target_or_host])

AC_MSG_CHECKING([for Win32 platform])
case "$target_or_host" in
  *-*-mingw* | *-*-cygwin*)
    os_win32=yes
    ;;
  *)
    os_win32=no
    ;;
esac
AC_MSG_RESULT([$os_win32])
AM_CONDITIONAL(OS_WIN32, test "$os_win32" = "yes")


#GNOME_COMPILE_WARNINGS

AC_CONFIG_FILES([po/Makefile.in
Makefile
src/Makefile
pixmaps/Makefile
])
AC_OUTPUT

# morris.spec
