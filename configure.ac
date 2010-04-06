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
AC_CHECK_LIB([alure], [alureCreateStreamFromMemory])
AC_CHECK_LIB([png], [png_sig_cmp])
AC_CHECK_LIB([glee], [GLeeInit])
AC_CHECK_LIB([GLee], [GLeeInit])

# Checks for header files.
AC_CHECK_HEADERS([limits.h malloc.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])
AC_CHECK_HEADERS([GL/glu.h SDL/SDL.h SDL/SDL_syswm.h png.h GLee.h AL/alure.h getopt.h])

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
if [$HAVE_LIBGL]
  then
    echo "OpenGL          ...found"
  else
    echo "OpenGL          ...not found"
fi
if [$HAVE_LIBGLU]
  then
    echo "libGLU          ...found"
  else
    echo "libGLU          ...not found"
fi
if [$HAVE_LIBSDL]
  then
    echo "SDL             ...found"
  else
    echo "SDL             ...not found"
fi
if [$HAVE_LIBPNG]
  then
    echo "libpng          ...found"
  else
    echo "libpng          ...not found"
fi
if [$HAVE_LIBGLEE]
  then
    echo "GLee            ...found"
  else
    echo "GLee            ...not found"
fi
if [$HAVE_LIBALURE]
  then
    echo "Alure (> 1.0)   ...found"
  else
    echo "Alure (> 1.0)   ...not found"
fi
echo ""
if [$HAVE_GL_GLU_H]
  then
    echo "GL/glu.h        ...found"
  else
    echo "GL/glu.h        ...not found"
fi
if [$HAVE_SDL_SDL_H]
  then
    echo "SDL/SDL.h       ...found"
  else
    echo "SDL/SDL.h       ...not found"
fi
if [$HAVE_SDL_SDL_SYSWM_H]
  then
    echo "SDL/SDL_syswm.h ...found"
  else
    echo "SDL/SDL_syswm.h ...not found"
fi
if [$HAVE_PNG_H]
  then
    echo "png.h           ...found"
  else
    echo "png.h           ...not found"
fi
if [$HAVE_GLEE_H]
  then
    echo "GLee.h          ...found"
  else
    echo "GLee.h          ...not found"
fi
if [$HAVE_AL_ALURE_H]
  then
    echo "AL/alure.h      ...found"
  else
    echo "AL/alure.h      ...not found"
fi
if [$HAVE_GETOPT_H]
  then
    echo "getopt.h        ...found"
  else
    echo "getopt.h        ...not found"
fi
echo "
Note that Alure needs VorbisFile, FLAC and DUMB to provide full sound support.
"
