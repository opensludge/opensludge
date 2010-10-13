#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.55])
AC_INIT([SLUDGE], [2.0b], [tobias.han@gmx.de], [sludge], [http://opensludge.sourceforge.net/])
AC_CONFIG_SRCDIR([source/CommonCode/tga.cpp])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
PKG_PROG_PKG_CONFIG()

AC_ARG_ENABLE([engine], [AC_HELP_STRING([--disable-engine], [don't build the SLUDGE Engine])])
AC_ARG_ENABLE([devkit], [AC_HELP_STRING([--enable-devkit], [build the SLUDGE DevKit])])

# Set default targets:
if test "x$enable_engine" = x; then
  enable_engine="yes"
fi
if test "x$enable_devkit" = x; then
  enable_devkit="no"
fi

# Are we cross compiling for windows?
case "${host}" in
        i[[3456789]]86-mingw32*) WIN32="yes" ;;
        *cygwin*) WIN32="yes" ;;
        *) WIN32="no" ;;
esac
AM_CONDITIONAL([COND_WIN32], test x"$WIN32" = xyes)

# Checks for libraries.
AC_CHECK_LIB([glee], [GLeeInit])
AC_CHECK_LIB([GLee], [GLeeInit])

if test "x$enable_engine" = xyes; then
  AC_CHECK_LIB([GL], [glBegin])
  AC_CHECK_LIB([GLU], [gluCylinder])
  AC_CHECK_LIB([SDL], [SDL_Init])
  AC_CHECK_LIB([alure], [alureUpdateInterval])
fi

if test "x$enable_devkit" = xyes; then
  PKG_CHECK_MODULES([GTK], [gtk+-2.0 gthread-2.0 gobject-2.0 gtkglext-1.0 gmodule-2.0])
fi

PKG_CHECK_MODULES([LIBPNG], [libpng])

# Checks for header files.
AC_CHECK_HEADERS([limits.h malloc.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])
AC_CHECK_HEADERS([GLee.h getopt.h])

if test "x$enable_engine" = xyes; then
  AC_CHECK_HEADERS([GL/glu.h SDL/SDL.h SDL/SDL_syswm.h AL/alure.h])
fi

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

AM_CONDITIONAL([COND_ENGINE], [test "x$enable_engine" = xyes])
AM_CONDITIONAL([COND_DEVKIT], [test "x$enable_devkit" = xyes])

AC_CONFIG_FILES([Makefile
                 source/Makefile
                 source/Engine/Makefile
                 GTK_Dev_Kit/Makefile])
AC_OUTPUT

echo "
SLUDGE configuration:

Please make sure that the following libraries and headers are installed:
"
if test $ac_cv_lib_GLee_GLeeInit; then
  if test "x$ac_cv_lib_glee_GLeeInit" = xyes; then
    echo "GLee            ...yes"
  else
    echo "GLee            ..."$ac_cv_lib_GLee_GLeeInit
  fi
else
    echo "GLee            ..."$ac_cv_lib_glee_GLeeInit
fi
echo ""
echo "GLee.h          ..."$ac_cv_header_GLee_h
echo "getopt.h        ..."$ac_cv_header_getopt_h

if test "x$enable_engine" = xyes; then
echo "
Additional requirements for the SLUDGE Engine:
"
echo "OpenGL          ..."$ac_cv_lib_GL_glBegin
echo "libGLU          ..."$ac_cv_lib_GLU_gluCylinder
echo "SDL             ..."$ac_cv_lib_SDL_SDL_Init
echo "Alure (>= 1.1)  ..."$ac_cv_lib_alure_alureUpdateInterval
echo ""
echo "GL/glu.h        ..."$ac_cv_header_GL_glu_h
echo "SDL/SDL.h       ..."$ac_cv_header_SDL_SDL_h
echo "SDL/SDL_syswm.h ..."$ac_cv_header_SDL_SDL_syswm_h
echo "AL/alure.h      ..."$ac_cv_header_AL_alure_h
echo "
Note that Alure needs VorbisFile, FLAC and DUMB to provide full sound support.
"
fi

echo "
Will build...
"
echo "SLUDGE Engine   ..."$enable_engine
echo "SLUDGE DevKit   ..."$enable_devkit
echo "
Use the options --disable-engine and/or --enable-devkit to (de)select components.
"
