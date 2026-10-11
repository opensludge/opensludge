cmake_minimum_required(VERSION 3.24)

project(OpenSLUDGE VERSION 2.3.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SLUDGE_GLES2 "Render with OpenGL ES 2.0 instead of desktop OpenGL" OFF)
set(SLUDGE_DEVICE "" CACHE STRING "Build for a device: OpenPandora, RaspberryPi or ODROID")
set_property(CACHE SLUDGE_DEVICE PROPERTY STRINGS "" OpenPandora RaspberryPi ODROID)
option(SLUDGE_MOVIES "Support playing WebM movies (needs libvpx)" ON)
option(SLUDGE_BUNDLED_DEPS "Build all dependencies from source and link them statically" OFF)
option(SLUDGE_TESTS "Build the tests" ON)

include(cmake/Dependencies.cmake)

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
	backdrop.cpp bg_effects.cpp builtin.cpp controllermap.cpp controllersettings.cpp cursors.cpp debug.cpp
	fileset.cpp floor.cpp fonttext.cpp freeze.cpp gamepad.cpp graphics.cpp helpers.cpp keys.cpp
	language.cpp line.cpp loadsave.cpp main.cpp pointer.cpp
	memwatch.cpp moreio.cpp newfatal.cpp objtypes.cpp people.cpp
	region.cpp savedata.cpp shaders.cpp sludge_gl.cpp sludger.cpp sprbanks.cpp
	sprites.cpp statusba.cpp stringy.cpp talk.cpp
	thumbnail.cpp timing.cpp transition.cpp variable.cpp
	zbuffer.cpp sound_sdl.cpp
)
if(SLUDGE_MOVIES)
	list(APPEND ENGINE_SOURCES movie.cpp libwebm/mkvparser.cpp libwebm/mkvreader.cpp)
else()
	list(APPEND ENGINE_SOURCES movie_none.cpp)
endif()
list(TRANSFORM ENGINE_SOURCES PREPEND ${ENGINE_DIR}/)
list(APPEND ENGINE_SOURCES ${COMMON_DIR}/utf8.cpp)

if(WIN32)
	# The resource script contains the setup dialog, the cursors and the icons.
	enable_language(RC)
	list(APPEND ENGINE_SOURCES ${ENGINE_DIR}/winstuff.cpp ${ENGINE_DIR}/dxgipresent.cpp ${CMAKE_CURRENT_SOURCE_DIR}/images/SLUDGE.RC)
else()
	list(APPEND ENGINE_SOURCES ${ENGINE_DIR}/posixstuff.cpp)
endif()

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

target_include_directories(sludge-engine PRIVATE ${GENERATED_DIR} ${COMMON_DIR} ${ENGINE_DIR}/libwebm)
target_link_libraries(sludge-engine PRIVATE
	sludge::sdl3 sludge::vorbis sludge::png sludge::dumb)

include(GNUInstallDirs)

if(SLUDGE_GLES2)
	target_compile_definitions(sludge-engine PRIVATE HAVE_GLES2 ${DEVICE_DEFINITIONS})
	target_link_libraries(sludge-engine PRIVATE GLESv2 ${DEVICE_LIBRARIES})
else()
	find_package(OpenGL REQUIRED)
	target_link_libraries(sludge-engine PRIVATE OpenGL::GL)
endif()

if(SLUDGE_MOVIES)
	target_link_libraries(sludge-engine PRIVATE sludge::vpx)
	if(TARGET libvpx)
		add_dependencies(sludge-engine libvpx)
	endif()
endif()

if(WIN32)
	set_target_properties(sludge-engine PROPERTIES WIN32_EXECUTABLE TRUE)
	target_include_directories(sludge-engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/images)
	target_compile_definitions(sludge-engine PRIVATE UNICODE _UNICODE)
	target_link_libraries(sludge-engine PRIVATE comdlg32 shell32 ole32)
	if(MINGW)
		# SDL provides wWinMain in Unicode builds, which requires -municode.
		# We link the C and C++ runtime libraries statically, so that the
		# engine runs without the MinGW DLLs.
		target_link_options(sludge-engine PRIVATE -municode -static -static-libgcc -static-libstdc++)
	endif()
endif()

# On Linux, iconv is part of the C library. On macOS, it is a separate
# library. On Windows, we convert text with the Windows API instead.
include(CheckFunctionExists)
check_function_exists(iconv_open HAVE_ICONV_IN_LIBC)
if(NOT WIN32 AND NOT HAVE_ICONV_IN_LIBC)
	find_package(Iconv REQUIRED)
	target_link_libraries(sludge-engine PRIVATE Iconv::Iconv)
endif()

install(TARGETS sludge-engine RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(SLUDGE_TESTS)
	enable_testing()
	add_executable(gamepad_test tests/gamepad_test.cpp ${ENGINE_DIR}/gamepad.cpp
		${ENGINE_DIR}/controllermap.cpp ${ENGINE_DIR}/keys.cpp)
	target_include_directories(gamepad_test PRIVATE ${ENGINE_DIR} ${COMMON_DIR})
	target_link_libraries(gamepad_test PRIVATE sludge::sdl3)
	add_test(NAME gamepad COMMAND gamepad_test)
endif()
