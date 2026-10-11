cmake_minimum_required(VERSION 3.24)

project(OpenSLUDGE VERSION 2.3.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SLUDGE_GLES2 "Render with OpenGL ES 2.0 instead of desktop OpenGL" OFF)
set(SLUDGE_DEVICE "" CACHE STRING "Build for a device: OpenPandora, RaspberryPi or ODROID")
set_property(CACHE SLUDGE_DEVICE PROPERTY STRINGS "" OpenPandora RaspberryPi ODROID)

find_package(SDL3 REQUIRED CONFIG)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ENGINE REQUIRED IMPORTED_TARGET vorbisfile vorbis ogg vpx libpng)
find_path(DUMB_INCLUDE_DIR dumb.h REQUIRED)
find_library(DUMB_LIBRARY dumb REQUIRED)

# On every device, we draw with OpenGL ES 2.0. On the OpenPandora, we always
# show the game in full screen, and we wait for the vertical sync of the
# framebuffer ourselves, because the graphics driver ignores the swap
# interval.
if(SLUDGE_DEVICE STREQUAL "OpenPandora")
	set(DEVICE_DEFINITIONS SLUDGE_ALWAYS_FULLSCREEN SLUDGE_FRAMEBUFFER_VSYNC)
	set(DEVICE_LIBRARIES iconv)
elseif(SLUDGE_DEVICE STREQUAL "RaspberryPi")
	# SDL 3 draws through the Broadcom driver of the Raspberry Pi when SDL
	# is built with bcm_host, and through Mesa otherwise.
elseif(SLUDGE_DEVICE STREQUAL "ODROID")
elseif(NOT SLUDGE_DEVICE STREQUAL "")
	message(FATAL_ERROR "SLUDGE_DEVICE must be OpenPandora, RaspberryPi or ODROID")
endif()
if(SLUDGE_DEVICE)
	set(SLUDGE_GLES2 ON)
endif()

set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source/Engine)
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source/CommonCode)

set(ENGINE_SOURCES
	backdrop.cpp bg_effects.cpp builtin.cpp cursors.cpp debug.cpp
	fileset.cpp floor.cpp fonttext.cpp freeze.cpp graphics.cpp helpers.cpp
	language.cpp line.cpp linuxstuff.cpp loadsave.cpp main.cpp
	memwatch.cpp moreio.cpp movie.cpp newfatal.cpp objtypes.cpp people.cpp
	region.cpp savedata.cpp shaders.cpp sludge_gl.cpp sludger.cpp sprbanks.cpp
	sprites.cpp statusba.cpp stringy.cpp talk.cpp
	thumbnail.cpp timing.cpp transition.cpp variable.cpp
	zbuffer.cpp sound_sdl.cpp
	libwebm/mkvparser.cpp libwebm/mkvreader.cpp
)
list(TRANSFORM ENGINE_SOURCES PREPEND ${ENGINE_DIR}/)
list(APPEND ENGINE_SOURCES ${COMMON_DIR}/utf8.cpp)

set(SHADERS
	fixScaleSprite.frag fixScaleSprite.vert
	scale.frag scale_noaa.frag scale.vert
	texture.frag texture.vert
	color.frag color.vert
	yuv.frag yuv.vert
)
list(TRANSFORM SHADERS PREPEND ${ENGINE_DIR}/)

# We build the shaders into the executable.
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
add_custom_command(
	OUTPUT ${GENERATED_DIR}/shaders_embedded.h
	COMMAND ${CMAKE_COMMAND} -DOUTPUT=${GENERATED_DIR}/shaders_embedded.h "-DSHADERS=${SHADERS}"
		-P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShaders.cmake
	DEPENDS ${SHADERS} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShaders.cmake
	COMMENT "Embedding shaders"
	VERBATIM)

add_executable(sludge-engine ${ENGINE_SOURCES} ${GENERATED_DIR}/shaders_embedded.h)

target_include_directories(sludge-engine PRIVATE ${GENERATED_DIR} ${COMMON_DIR} ${ENGINE_DIR}/libwebm ${DUMB_INCLUDE_DIR})
target_link_libraries(sludge-engine PRIVATE SDL3::SDL3 PkgConfig::ENGINE ${DUMB_LIBRARY})

include(GNUInstallDirs)

if(SLUDGE_GLES2)
	target_compile_definitions(sludge-engine PRIVATE HAVE_GLES2 ${DEVICE_DEFINITIONS})
	target_link_libraries(sludge-engine PRIVATE GLESv2 ${DEVICE_LIBRARIES})
else()
	find_package(OpenGL REQUIRED)
	target_link_libraries(sludge-engine PRIVATE OpenGL::GL)
endif()

install(TARGETS sludge-engine RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
