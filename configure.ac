#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
# This builds the SLUDGE Development Kit and the documentation. We build the
# engine with CMake (see CMakeLists.txt).

AC_PREREQ([2.55])
AC_INIT([SLUDGE], [2.3.0], [tobias.han@gmx.de], [sludge], [http://opensludge.github.io/])
AC_CONFIG_SRCDIR([source/CommonCode/tga.cpp])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([config.h])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_CANONICAL_HOST
PKG_PROG_PKG_CONFIG()

AC_ARG_ENABLE([devkit], [AS_HELP_STRING([--disable-devkit], [don't build the SLUDGE DevKit])])
AC_ARG_ENABLE([doc], [AS_HELP_STRING([--enable-doc], [install the SLUDGE documentation])])

# Set default targets:
if test "x$enable_devkit" = x; then
  enable_devkit="yes"
fi
if test "x$enable_doc" = x; then
  enable_doc="no"
fi

# Are we cross compiling for windows?
case "${host}" in
        i[[3456789]]86-mingw32*) WIN32="yes" ;;
        *cygwin*) WIN32="yes" ;;
        *) WIN32="no" ;;
esac
AM_CONDITIONAL([COND_WIN32], test x"$WIN32" = xyes)

# Checks for libraries.
if test "x$enable_devkit" = xyes; then
  PKG_CHECK_MODULES([GLEW], [glew])
  PKG_CHECK_MODULES([GTK], [gtk+-3.0 >= 3.16 gthread-2.0 gobject-2.0 gmodule-2.0])
  PKG_CHECK_MODULES([GLIB], [glib-2.0])
  PKG_CHECK_MODULES([LIBPNG], [libpng])
fi

# Checks for header files.
AC_CHECK_HEADERS([limits.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h], [], [AC_MSG_ERROR([cannot find one or more of these header files: limits.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])])
AC_CHECK_HEADERS([getopt.h], [], [AC_MSG_ERROR([cannot find getopt header files])])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([atexit getcwd memset mkdir rmdir sqrt strstr strtol])

AM_CONDITIONAL([COND_DEVKIT], [test "x$enable_devkit" = xyes])
AM_CONDITIONAL([COND_DOC], [test "x$enable_doc" = xyes])

AC_CONFIG_FILES([Makefile
                 GTK_Dev_Kit/Makefile
                 GTK_Dev_Kit/support/Makefile
                 doc/Makefile])
AC_OUTPUT

echo "
SLUDGE configuration complete.

Will build..."
if test x"$WIN32" = xyes; then
echo "...the SLUDGE DevKit for Windows"
else
echo ""
echo "SLUDGE DevKit        ..."$enable_devkit
echo "SLUDGE documentation ..."$enable_doc
echo "
Use the options --disable-devkit and/or --enable-doc to (de)select components.
Build the SLUDGE engine with CMake: see INSTALL.
"
fi
