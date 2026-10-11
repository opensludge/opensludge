# Cross-compile the engine for 32-bit Windows, from Windows XP on, with
# MinGW-w64 on Linux.
#
# We use the compilers with the win32 thread model, because the POSIX
# threads library of MinGW-w64 calls GetTickCount64, which Windows XP does
# not have. In CI, we check the executable with
# packaging/windows/check-xp-imports.py.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86)
set(CMAKE_C_COMPILER i686-w64-mingw32-gcc-win32)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++-win32)
set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
set(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
