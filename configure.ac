#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.55])
AC_INIT([opensludge-engine], [2.0b], [tobias.han@gmx.de])
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

# Checks for header files.
AC_CHECK_HEADERS([limits.h malloc.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])

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
