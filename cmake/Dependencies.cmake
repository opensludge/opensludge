# Find or build the libraries of the engine, and make a target for each of
# them, as follows.
#
#   sludge::sdl3   SDL 3
#   sludge::vorbis libvorbisfile (with libvorbis and libogg)
#   sludge::png    libpng
#   sludge::dumb   DUMB (tracker music)
#   sludge::vpx    libvpx (for movies, only when SLUDGE_MOVIES is on)
#
# By default, we use the libraries installed on the system, and we build
# from source those that we don't find. With SLUDGE_BUNDLED_DEPS, we build
# every library from source and link it statically, so that the executable
# runs without them.

include(FetchContent)

# The CMake files of some of these libraries declare a minimum CMake version
# older than 3.5. CMake 4 stops with an error on those unless we set this.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(SLUDGE_SDL_TAG release-3.4.18)
set(SLUDGE_OGG_TAG v1.3.6)
set(SLUDGE_VORBIS_TAG v1.3.7)
set(SLUDGE_PNG_TAG v1.6.59)
set(SLUDGE_ZLIB_TAG v1.3.2)
set(SLUDGE_DUMB_TAG 2.0.3)
set(SLUDGE_VPX_TAG v1.17.0)

if(NOT SLUDGE_BUNDLED_DEPS)
	find_package(PkgConfig QUIET)
endif()

# Build a library from source as a static library. We don't install it, and
# we don't build its tests, examples and tools.
macro(sludge_fetch name repository tag)
	set(BUILD_SHARED_LIBS OFF)
	set(BUILD_TESTING OFF)
	FetchContent_Declare(${name}
		GIT_REPOSITORY ${repository}
		GIT_TAG ${tag}
		GIT_SHALLOW TRUE
		${ARGN})
	FetchContent_MakeAvailable(${name})
endmacro()

# Make the target sludge::<name>, which links to the given libraries.
function(sludge_target name)
	add_library(sludge_${name} INTERFACE)
	target_link_libraries(sludge_${name} INTERFACE ${ARGN})
	add_library(sludge::${name} ALIAS sludge_${name})
endfunction()

#
# SDL 3
#
if(NOT SLUDGE_BUNDLED_DEPS)
	find_package(SDL3 CONFIG QUIET)
endif()
if(TARGET SDL3::SDL3)
	message(STATUS "Using system SDL ${SDL3_VERSION}")
	sludge_target(sdl3 SDL3::SDL3)
else()
	message(STATUS "Building SDL from source")
	set(SDL_SHARED OFF CACHE BOOL "" FORCE)
	set(SDL_STATIC ON CACHE BOOL "" FORCE)
	set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
	set(SDL_TESTS OFF CACHE BOOL "" FORCE)
	set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
	set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
	sludge_fetch(SDL3 https://github.com/libsdl-org/SDL.git ${SLUDGE_SDL_TAG})
	sludge_target(sdl3 SDL3-static)
endif()

#
# Ogg Vorbis
#
if(NOT SLUDGE_BUNDLED_DEPS AND PkgConfig_FOUND)
	pkg_check_modules(VORBISFILE QUIET IMPORTED_TARGET vorbisfile vorbis ogg)
endif()
if(TARGET PkgConfig::VORBISFILE)
	message(STATUS "Using system libvorbis")
	sludge_target(vorbis PkgConfig::VORBISFILE)
else()
	message(STATUS "Building libogg and libvorbis from source")
	set(INSTALL_DOCS OFF CACHE BOOL "" FORCE)
	# With OVERRIDE_FIND_PACKAGE, find_package(Ogg) in the libvorbis CMake
	# files finds the libogg that we build here.
	sludge_fetch(Ogg https://github.com/xiph/ogg.git ${SLUDGE_OGG_TAG} OVERRIDE_FIND_PACKAGE)
	sludge_fetch(Vorbis https://github.com/xiph/vorbis.git ${SLUDGE_VORBIS_TAG})
	sludge_target(vorbis vorbisfile vorbis ogg)
endif()

#
# libpng
#
if(NOT SLUDGE_BUNDLED_DEPS)
	find_package(PNG QUIET)
endif()
if(TARGET PNG::PNG)
	message(STATUS "Using system libpng")
	sludge_target(png PNG::PNG)
else()
	message(STATUS "Building libpng from source")
	# zlib is part of macOS and of every Linux system, so we link to the
	# system zlib there. On Windows, we build it from source when we bundle
	# the libraries, because Windows has no zlib.
	if(NOT (SLUDGE_BUNDLED_DEPS AND WIN32))
		find_package(ZLIB QUIET)
	endif()
	if(NOT ZLIB_FOUND)
		message(STATUS "Building zlib from source")
		set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
		set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
		set(ZLIB_BUILD_SHARED OFF CACHE BOOL "" FORCE)
		set(ZLIB_BUILD_STATIC ON CACHE BOOL "" FORCE)
		set(ZLIB_INSTALL OFF CACHE BOOL "" FORCE)
		set(SKIP_INSTALL_ALL ON)
		sludge_fetch(ZLIB https://github.com/madler/zlib.git ${SLUDGE_ZLIB_TAG} OVERRIDE_FIND_PACKAGE)
		target_include_directories(zlibstatic INTERFACE ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
		if(NOT TARGET ZLIB::ZLIB)
			add_library(ZLIB::ZLIB ALIAS zlibstatic)
		endif()
		set(ZLIB_LIBRARIES zlibstatic)
		set(ZLIB_INCLUDE_DIRS ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
		set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR})
	endif()
	set(PNG_SHARED OFF CACHE BOOL "" FORCE)
	set(PNG_STATIC ON CACHE BOOL "" FORCE)
	set(PNG_TESTS OFF CACHE BOOL "" FORCE)
	set(PNG_TOOLS OFF CACHE BOOL "" FORCE)
	set(PNG_FRAMEWORK OFF CACHE BOOL "" FORCE)
	# libpng chooses its SIMD code for the processor of the build machine,
	# which is wrong when we build for the other architecture on a Mac.
	set(PNG_HARDWARE_OPTIMIZATIONS OFF CACHE BOOL "" FORCE)
	set(SKIP_INSTALL_ALL ON)
	sludge_fetch(libpng https://github.com/pnggroup/libpng.git ${SLUDGE_PNG_TAG})
	sludge_target(png png_static ZLIB::ZLIB)
	target_include_directories(sludge_png INTERFACE ${libpng_SOURCE_DIR} ${libpng_BINARY_DIR})
endif()

#
# DUMB
#
if(NOT SLUDGE_BUNDLED_DEPS)
	find_path(DUMB_INCLUDE_DIR dumb.h)
	find_library(DUMB_LIBRARY dumb)
endif()
if(DUMB_INCLUDE_DIR AND DUMB_LIBRARY)
	message(STATUS "Using system DUMB")
	sludge_target(dumb ${DUMB_LIBRARY})
	target_include_directories(sludge_dumb INTERFACE ${DUMB_INCLUDE_DIR})
else()
	message(STATUS "Building DUMB from source")
	set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	set(BUILD_ALLEGRO4 OFF CACHE BOOL "" FORCE)
	sludge_fetch(dumb https://github.com/kode54/dumb.git ${SLUDGE_DUMB_TAG})
	sludge_target(dumb dumb)
	target_include_directories(sludge_dumb INTERFACE ${dumb_SOURCE_DIR}/include)
	# DUMB 2 requires a 64-bit off_t. With MinGW, off_t is only 64 bits
	# wide with this definition.
	target_compile_definitions(sludge_dumb INTERFACE _FILE_OFFSET_BITS=64)
endif()

#
# libvpx (for movies)
#
if(SLUDGE_MOVIES)
	if(NOT SLUDGE_BUNDLED_DEPS AND PkgConfig_FOUND)
		pkg_check_modules(VPX QUIET IMPORTED_TARGET vpx)
	endif()
	if(TARGET PkgConfig::VPX)
		message(STATUS "Using system libvpx")
		sludge_target(vpx PkgConfig::VPX)
	elseif(MSVC)
		message(FATAL_ERROR "libvpx was not found. Install it, or turn off SLUDGE_MOVIES.")
	else()
		message(STATUS "Building libvpx from source")
		include(ExternalProject)

		# libvpx has a configure script instead of CMake files. We use only
		# the VP8 decoder.
		set(vpx_options
			--disable-shared --enable-static --enable-pic
			--disable-examples --disable-tools --disable-docs --disable-unit-tests
			--disable-vp9 --disable-vp8-encoder --enable-vp8-decoder)
		set(vpx_env)
		if(APPLE)
			list(LENGTH CMAKE_OSX_ARCHITECTURES num_archs)
			if(num_archs GREATER 1)
				message(FATAL_ERROR "Build one architecture at a time and combine the executables with lipo.")
			endif()
			if(CMAKE_OSX_ARCHITECTURES)
				set(vpx_arch ${CMAKE_OSX_ARCHITECTURES})
			else()
				set(vpx_arch ${CMAKE_HOST_SYSTEM_PROCESSOR})
			endif()
			if(vpx_arch STREQUAL "arm64")
				list(APPEND vpx_options --target=arm64-darwin20-gcc)
			else()
				list(APPEND vpx_options --target=x86_64-darwin20-gcc)
			endif()
			set(vpx_flags "-arch ${vpx_arch}")
			if(CMAKE_OSX_DEPLOYMENT_TARGET)
				string(APPEND vpx_flags " -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
			endif()
			list(APPEND vpx_options "--extra-cflags=${vpx_flags}" "--extra-cxxflags=${vpx_flags}")
			set(vpx_env "LDFLAGS=${vpx_flags}")
		endif()
		if(MINGW)
			# SLUDGE movies are small, so we decode them on one thread, and
			# we build libvpx without a POSIX threads library on Windows.
			list(APPEND vpx_options --disable-multithread)
			if(CMAKE_SIZEOF_VOID_P EQUAL 8)
				list(APPEND vpx_options --target=x86_64-win64-gcc)
			elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
				list(APPEND vpx_options --target=x86-win32-gcc)
			else()
				message(FATAL_ERROR "No libvpx target for MinGW with ${CMAKE_SIZEOF_VOID_P}-byte pointers")
			endif()
			if(CMAKE_CROSSCOMPILING)
				# e.g. x86_64-w64-mingw32-gcc-posix -> x86_64-w64-mingw32-
				get_filename_component(vpx_cc ${CMAKE_C_COMPILER} NAME)
				string(REGEX REPLACE "gcc.*$" "" vpx_cross ${vpx_cc})
				list(APPEND vpx_env "CROSS=${vpx_cross}")
			endif()
		endif()
		# We assemble the x86 SIMD code of libvpx with nasm.
		find_program(NASM_EXECUTABLE nasm)
		if(NASM_EXECUTABLE)
			list(APPEND vpx_options --as=nasm)
		endif()

		# The configure script is a shell script. On Windows, we run it with
		# the MSYS2 shell, because cmd.exe cannot run it.
		set(vpx_shell)
		if(CMAKE_HOST_WIN32)
			find_program(SH_EXECUTABLE sh REQUIRED)
			set(vpx_shell ${SH_EXECUTABLE})
		endif()
		find_program(MAKE_EXECUTABLE NAMES gmake make REQUIRED)

		set(vpx_prefix ${CMAKE_CURRENT_BINARY_DIR}/libvpx)
		ExternalProject_Add(libvpx
			GIT_REPOSITORY https://github.com/webmproject/libvpx.git
			GIT_TAG ${SLUDGE_VPX_TAG}
			GIT_SHALLOW TRUE
			PREFIX ${vpx_prefix}
			CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env ${vpx_env} CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
				${vpx_shell} <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> ${vpx_options}
			BUILD_COMMAND ${MAKE_EXECUTABLE} -j8
			INSTALL_COMMAND ${MAKE_EXECUTABLE} install
			BUILD_BYPRODUCTS ${vpx_prefix}/lib/libvpx.a)
		file(MAKE_DIRECTORY ${vpx_prefix}/include)
		add_library(vpx_static STATIC IMPORTED)
		set_target_properties(vpx_static PROPERTIES
			IMPORTED_LOCATION ${vpx_prefix}/lib/libvpx.a
			INTERFACE_INCLUDE_DIRECTORIES ${vpx_prefix}/include)
		sludge_target(vpx vpx_static)
	endif()
endif()
