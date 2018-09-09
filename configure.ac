#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.55])
AC_INIT([SLUDGE], [2.2.1], [tobias.han@gmx.de], [sludge], [http://opensludge.github.io/])
AC_CONFIG_SRCDIR([source/CommonCode/tga.cpp])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([config.h])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
PKG_PROG_PKG_CONFIG()

AC_ARG_ENABLE([engine], [AC_HELP_STRING([--disable-engine], [don't build the SLUDGE Engine])])
AC_ARG_ENABLE([devkit], [AC_HELP_STRING([--enable-devkit], [build the SLUDGE DevKit])])
AC_ARG_ENABLE([doc], [AC_HELP_STRING([--enable-doc], [install the SLUDGE documentation])])
AC_ARG_ENABLE([gles2], [AC_HELP_STRING([--enable-gles2], [build engine using OpenGL ES 2.0])])
AC_ARG_ENABLE([pandora], [AC_HELP_STRING([--enable-pandora], [enable hacks for OpenPandora])])
AC_ARG_ENABLE([rpi], [AC_HELP_STRING([--enable-rpi], [enable Raspberry-PI support])])
AC_ARG_ENABLE([odroid], [AC_HELP_STRING([--enable-odroid], [enable ODroid support])])

# Set default targets:
if test "x$enable_engine" = x; then
  enable_engine="yes"
fi
if test "x$enable_devkit" = x; then
  enable_devkit="no"
fi
if test "x$enable_doc" = x; then
  enable_doc="no"
fi
if test "x$enable_gles2" = x; then
  enable_gles2="no"
fi
if test "x$enable_pandora" = x; then
  enable_pandora="no"
fi
if test "x$enable_rpi" = x; then
  enable_rpi="no"
fi
if test "x$enable_odroid" = x; then
  enable_odroid="no"
fi
if test "x$enable_pandora" = xyes; then
  enable_gles2="yes"
fi
if test "x$enable_rpi" = xyes; then
  enable_gles2="yes"
fi
if test "x$enable_odroid" = xyes; then
  enable_gles2="yes"
fi

# Are we cross compiling for windows?
case "${host}" in
        i[[3456789]]86-mingw32*) WIN32="yes" ;;
        *cygwin*) WIN32="yes" ;;
        *) WIN32="no" ;;
esac
AM_CONDITIONAL([COND_WIN32], test x"$WIN32" = xyes)

# Always build the Dev Kit for windows:
if test x"$WIN32" = xyes; then
  enable_engine="no"
  enable_devkit="yes"
fi

# Checks for libraries.
if test "x$enable_gles2" = xno; then
  PKG_CHECK_MODULES([GLEW], [glew])
else
  if test "x$enable_devkit" = xyes; then
    PKG_CHECK_MODULES([GLEW], [glew])
  fi
fi

if test "x$enable_engine" = xyes; then
  PKG_CHECK_MODULES([ENGINE], [sdl alure >= 1.1 openal vorbis ogg vpx])
  if test "x$enable_gles2" = xyes; then
    AC_CHECK_LIB([GLESv2], [glEnable], [ENGINE_LIBS="$ENGINE_LIBS -lGLESv2"], [AC_MSG_ERROR([cannot find GLESv2 library])])
    AC_CHECK_LIB([EGL], [eglInitialize], [ENGINE_LIBS="$ENGINE_LIBS -lEGL"], [AC_MSG_ERROR([cannot find EGL library])])
    AC_CHECK_LIB([X11], [XOpenDisplay], [ENGINE_LIBS="$ENGINE_LIBS -lX11"], [AC_MSG_ERROR([cannot find X11 library])])
  else
    PKG_CHECK_MODULES([GL], [gl glu])
  fi
fi

if test "x$enable_devkit" = xyes; then
  PKG_CHECK_MODULES([GTK], [gtk+-2.0 gthread-2.0 gobject-2.0 gtkglext-1.0 gmodule-2.0])
  PKG_CHECK_MODULES([GLIB], [glib-2.0])
fi

PKG_CHECK_MODULES([LIBPNG], [libpng])

# Checks for header files.
AC_CHECK_HEADERS([limits.h malloc.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h], [], [AC_MSG_ERROR([cannot find one or more of these header files: limits.h malloc.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])])
AC_CHECK_HEADERS([getopt.h], [], [AC_MSG_ERROR([cannot find getopt header files])])

if test "x$enable_engine" = xyes; then
  AC_CHECK_HEADERS([SDL/SDL.h SDL/SDL_syswm.h], [], [AC_MSG_ERROR([cannot find SDL header files])])
  AC_CHECK_HEADERS([AL/alure.h], [], [AC_MSG_ERROR([cannot find alure header files])])
  AC_CHECK_HEADERS([vpx/vpx_decoder.h], [], [AC_MSG_ERROR([cannot find vpx header files])])
  AC_CHECK_HEADERS([vorbis/codec.h], [], [AC_MSG_ERROR([cannot find vorbis header files])])
  AC_CHECK_HEADERS([ogg/ogg.h], [], [AC_MSG_ERROR([cannot find ogg header files])])
  if test "x$enable_gles2" = xyes; then
    AC_CHECK_HEADERS([GLES2/gl2.h], [], [AC_MSG_ERROR([cannot find gles2 header files])])
    AC_CHECK_HEADERS([EGL/egl.h], [], [AC_MSG_ERROR([cannot find egl header files])])
    AC_CHECK_HEADERS([X11/Xlib.h X11/Xutil.h], [], [AC_MSG_ERROR([cannot find x11 header files])])
  else
    AC_CHECK_HEADERS([GL/glu.h], [], [AC_MSG_ERROR([cannot find glu header files])])
  fi
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
AM_CONDITIONAL([COND_DOC], [test "x$enable_doc" = xyes])
AM_CONDITIONAL([COND_GLES2], [test "x$enable_gles2" = xyes])
AM_CONDITIONAL([COND_PANDORA], [test "x$enable_pandora" = xyes])
AM_CONDITIONAL([COND_RPI], [test "x$enable_rpi" = xyes])
AM_CONDITIONAL([COND_ODROID], [test "x$enable_odroid" = xyes])

AC_CONFIG_FILES([Makefile
                 source/Makefile
                 source/Engine/Makefile
                 GTK_Dev_Kit/Makefile
                 GTK_Dev_Kit/support/Makefile
                 doc/Makefile])
AC_OUTPUT

echo "
SLUDGE configuration complete.
Note that Alure needs VorbisFile, FLAC and DUMB to provide full sound support for the SLUDGE engine.

Will build..."
if test x"$WIN32" = xyes; then
echo "...the SLUDGE DevKit for Windows"
else
echo ""
echo "SLUDGE Engine        ..."$enable_engine
echo "SLUDGE DevKit        ..."$enable_devkit
echo "SLUDGE documentation ..."$enable_doc
echo "
Use the options --disable-engine, --enable-devkit and/or --enable-doc to (de)select components.
"
fi
