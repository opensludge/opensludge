cmake_minimum_required(VERSION 3.24)

project(OpenSLUDGE VERSION 2.3.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SLUDGE_GLES2 "Render with OpenGL ES 2.0 instead of desktop OpenGL" OFF)
set(SLUDGE_DEVICE "" CACHE STRING "Build for a device: OpenPandora, RaspberryPi or ODROID")
set_property(CACHE SLUDGE_DEVICE PROPERTY STRINGS "" OpenPandora RaspberryPi ODROID)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ENGINE REQUIRED IMPORTED_TARGET sdl alure>=1.1 openal vorbis ogg vpx libpng)
find_path(DUMB_INCLUDE_DIR dumb.h REQUIRED)
find_library(DUMB_LIBRARY dumb REQUIRED)

# On every device, we draw with OpenGL ES 2.0, and we build eglport with the
# definition and the libraries for that device.
if(SLUDGE_DEVICE STREQUAL "OpenPandora")
	set(DEVICE_DEFINITIONS PANDORA)
	set(DEVICE_LIBRARIES iconv)
elseif(SLUDGE_DEVICE STREQUAL "RaspberryPi")
	pkg_check_modules(BCM_HOST REQUIRED IMPORTED_TARGET bcm_host)
	set(DEVICE_DEFINITIONS RPI)
	set(DEVICE_LIBRARIES PkgConfig::BCM_HOST)
elseif(SLUDGE_DEVICE STREQUAL "ODROID")
	set(DEVICE_DEFINITIONS ODROID)
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
	region.cpp savedata.cpp shaders.cpp sludger.cpp sprbanks.cpp
	sprites.cpp statusba.cpp stringy.cpp talk.cpp
	thumbnail.cpp timing.cpp transition.cpp variable.cpp
	zbuffer.cpp sound_openal.cpp
	libwebm/mkvparser.cpp libwebm/mkvreader.cpp
)
if(SLUDGE_GLES2)
	list(APPEND ENGINE_SOURCES eglport/eglport.cpp)
endif()
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

add_executable(sludge-engine ${ENGINE_SOURCES})

target_include_directories(sludge-engine PRIVATE ${COMMON_DIR} ${ENGINE_DIR}/libwebm ${DUMB_INCLUDE_DIR})
target_link_libraries(sludge-engine PRIVATE PkgConfig::ENGINE ${DUMB_LIBRARY})

include(GNUInstallDirs)
# We load the shaders from this folder at run time.
target_compile_definitions(sludge-engine PRIVATE DATADIR="${CMAKE_INSTALL_FULL_DATADIR}/sludge/")

if(SLUDGE_GLES2)
	target_compile_definitions(sludge-engine PRIVATE HAVE_GLES2 USE_EGL_SDL USE_GLES2 ${DEVICE_DEFINITIONS})
	target_link_libraries(sludge-engine PRIVATE GLESv2 EGL X11 ${DEVICE_LIBRARIES})
else()
	find_package(OpenGL REQUIRED)
	pkg_check_modules(GLEW REQUIRED IMPORTED_TARGET glew glu)
	target_link_libraries(sludge-engine PRIVATE OpenGL::GL PkgConfig::GLEW)
endif()

install(TARGETS sludge-engine RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${SHADERS} DESTINATION ${CMAKE_INSTALL_DATADIR}/sludge)
