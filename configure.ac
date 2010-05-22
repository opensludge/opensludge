#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.55])
AC_INIT([sludge-engine], [2.0b], [tobias.han@gmx.de])
AC_CONFIG_SRCDIR([source/Engine/sludger.cpp])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB([GL], [glBegin])
AC_CHECK_LIB([GLU], [gluCylinder])
AC_CHECK_LIB([SDL], [SDL_Init])
AC_CHECK_LIB([alure], [alureUpdateInterval])
AC_CHECK_LIB([png], [png_sig_cmp])
AC_CHECK_LIB([glee], [GLeeInit])
AC_CHECK_LIB([GLee], [GLeeInit])

# Checks for header files.
AC_CHECK_HEADERS([limits.h malloc.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])
AC_CHECK_HEADERS([GL/glu.h SDL/SDL.h SDL/SDL_syswm.h png.h GLee.h AL/alure.h getopt.h])

# Check if libpng-config finds libpng.
mylibpngldflags=`libpng-config --ldflags`
if test $? -eq 0
  then
    if test ${mylibpngldflags%%\ *}
      then
        ac_cv_lib_png_png_sig_cmp="yes"
    fi
fi
mylibpngcflags=`libpng-config --cflags`
if test $? -eq 0
  then
    if test ${mylibpngcflags%%\ *}
      then
        ac_cv_header_png_h="yes"
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

AC_CONFIG_FILES([source/Engine/Makefile
                 source/Makefile
                 Makefile])
AC_OUTPUT

echo "
OpenSLUDGE Engine configuration:

Please make sure that the following libraries and headers are installed:
"
echo "OpenGL          ..."$ac_cv_lib_GL_glBegin
echo "libGLU          ..."$ac_cv_lib_GLU_gluCylinder
echo "SDL             ..."$ac_cv_lib_SDL_SDL_Init
echo "libpng          ..."$ac_cv_lib_png_png_sig_cmp
if test $ac_cv_lib_glee_GLeeInit
  then
    echo "GLee            ..."$ac_cv_lib_glee_GLeeInit
  else
    echo "GLee            ..."$ac_cv_lib_GLee_GLeeInit
fi
echo "Alure (>= 1.1)  ..."$ac_cv_lib_alure_alureUpdateInterval
echo ""
echo "GL/glu.h        ..."$ac_cv_header_GL_glu_h
echo "SDL/SDL.h       ..."$ac_cv_header_SDL_SDL_h
echo "SDL/SDL_syswm.h ..."$ac_cv_header_SDL_SDL_syswm_h
echo "png.h           ..."$ac_cv_header_png_h
echo "GLee.h          ..."$ac_cv_header_GLee_h
echo "AL/alure.h      ..."$ac_cv_header_AL_alure_h
echo "getopt.h        ..."$ac_cv_header_getopt_h
echo "
Note that Alure needs VorbisFile, FLAC and DUMB to provide full sound support.
"
