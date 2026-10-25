# The SLUDGE Dev Kit: the compiler, and the GTK 3 tools around it.

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED IMPORTED_TARGET gtk+-3.0>=3.20 gthread-2.0 gobject-2.0 gmodule-2.0)
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(EPOXY REQUIRED IMPORTED_TARGET epoxy)
pkg_check_modules(LIBPNG REQUIRED IMPORTED_TARGET libpng)

set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/source)
set(DEVKIT_DIR ${SOURCE_DIR}/DevKit)
set(IMAGES_DIR ${PROJECT_SOURCE_DIR}/images)
set(PKGDATADIR ${CMAKE_INSTALL_DATADIR}/sludge)

include_directories(
	${SOURCE_DIR}/CommonCode
	${DEVKIT_DIR}/Common
	${DEVKIT_DIR}/Compiler
	${DEVKIT_DIR}/ProjectManager
	${DEVKIT_DIR}/GraphicalTools
	${DEVKIT_DIR}/TranslationEditor
)

# The tools load their user interfaces and icons from DATADIR. On Windows, we
# put them in the resources folder next to the executables.
if(WIN32)
	enable_language(RC)
	add_compile_definitions(DATADIR="resources/")
else()
	add_compile_definitions(DATADIR="${CMAKE_INSTALL_FULL_DATADIR}/sludge/")
endif()

# The code that the tools share, in static libraries.
add_library(devkit-common STATIC
	${DEVKIT_DIR}/Common/moreio.cpp
	${DEVKIT_DIR}/Common/helpers.cpp
	${DEVKIT_DIR}/Common/stringarray.cpp
	${SOURCE_DIR}/CommonCode/tga.cpp
	${SOURCE_DIR}/CommonCode/utf8.cpp
)

add_library(devkit-compiler STATIC
	${DEVKIT_DIR}/ProjectManager/project.cpp
	${DEVKIT_DIR}/ProjectManager/settings.cpp
	${DEVKIT_DIR}/ProjectManager/messbox.cpp
	${DEVKIT_DIR}/Compiler/compiler.cpp
	${DEVKIT_DIR}/Compiler/compilerinfo.cpp
	${DEVKIT_DIR}/Compiler/allknown.cpp
	${DEVKIT_DIR}/Compiler/checkused.cpp
	${DEVKIT_DIR}/Compiler/dumpfiles.cpp
	${DEVKIT_DIR}/Compiler/floor.cpp
	${DEVKIT_DIR}/Compiler/hsi.cpp
	${DEVKIT_DIR}/Compiler/linker.cpp
	${DEVKIT_DIR}/Compiler/objtype.cpp
	${DEVKIT_DIR}/Compiler/preproc.cpp
	${DEVKIT_DIR}/Compiler/realproc.cpp
	${DEVKIT_DIR}/Compiler/sludge_functions.cpp
	${DEVKIT_DIR}/Compiler/splitter.cpp
	${DEVKIT_DIR}/Compiler/tokens.cpp
	${DEVKIT_DIR}/Compiler/translation.cpp
)
target_link_libraries(devkit-compiler PUBLIC devkit-common PkgConfig::LIBPNG)

# The drawing of the views of the floor maker, the z-buffer maker and the
# sprite bank editor.
add_library(devkit-graphics STATIC
	${DEVKIT_DIR}/GraphicalTools/draw2d.cpp
	${DEVKIT_DIR}/GraphicalTools/sprites.cpp
	${DEVKIT_DIR}/GraphicalTools/floormaker.cpp
	${DEVKIT_DIR}/GraphicalTools/zbuffer.cpp
)
target_link_libraries(devkit-graphics PUBLIC devkit-common PkgConfig::EPOXY PkgConfig::LIBPNG)

add_library(devkit-gtk STATIC SludgeApplication.cpp Common.cpp)
target_link_libraries(devkit-gtk PUBLIC devkit-common PkgConfig::GTK)

# The compiler runs without a display.
add_executable(sludge-compiler CompilerMain.cpp)
target_link_libraries(sludge-compiler PRIVATE devkit-compiler PkgConfig::GLIB)

# Add a GTK tool, with its main source file, the class of its window, its
# Windows resource script and any further source files.
function(add_devkit_tool name main window resources)
	add_executable(${name} ${main} ${window} ${ARGN})
	target_link_libraries(${name} PRIVATE devkit-gtk)
	if(WIN32)
		target_sources(${name} PRIVATE ${resources})
		set_target_properties(${name} PROPERTIES WIN32_EXECUTABLE TRUE)
	endif()
	install(TARGETS ${name} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endfunction()

add_devkit_tool(sludge-floormaker FloorMakerMain.cpp SludgeFloorMaker.cpp FloorMaker.rc
	SludgeGLApplication.cpp)
add_devkit_tool(sludge-zbuffermaker ZBufferMakerMain.cpp SludgeZBufferMaker.cpp ZBufferMaker.rc
	SludgeGLApplication.cpp)
add_devkit_tool(sludge-spritebankeditor SpriteBankEditorMain.cpp SludgeSpriteBankEditor.cpp SpriteBankEditor.rc
	SludgeGLApplication.cpp)
add_devkit_tool(sludge-translationeditor TranslationEditorMain.cpp SludgeTranslationEditor.cpp TranslationEditor.rc
	${DEVKIT_DIR}/TranslationEditor/translator.cpp)
add_devkit_tool(sludge-projectmanager ProjectManagerMain.cpp SludgeProjectManager.cpp ProjectManager.rc)

target_link_libraries(sludge-floormaker PRIVATE devkit-graphics)
target_link_libraries(sludge-zbuffermaker PRIVATE devkit-graphics)
target_link_libraries(sludge-spritebankeditor PRIVATE devkit-graphics)
target_link_libraries(sludge-projectmanager PRIVATE devkit-compiler)

install(TARGETS sludge-compiler RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES
	FloorMaker.glade ZBufferMaker.glade SpriteBankEditor.glade
	ProjectManager.glade TranslationEditor.glade
	${IMAGES_DIR}/floormodeicon1.png ${IMAGES_DIR}/floormodeicon2.png
	${IMAGES_DIR}/floormodeicon3.png ${IMAGES_DIR}/floormodeicon4.png
	${IMAGES_DIR}/floormodeicon5.png
	${IMAGES_DIR}/compiler/compiler_16x16x32.png
	${IMAGES_DIR}/gameIcon/gameIcon_16x16x32.png
	DESTINATION ${PKGDATADIR})
foreach(icon flags floorIcon zIcon spriteIcon ProjIcon)
	foreach(size 16x16 32x32 128x128 256x256)
		install(FILES ${IMAGES_DIR}/${icon}/${icon}_${size}x32.png DESTINATION ${PKGDATADIR})
	endforeach()
endforeach()

# The desktop integration: menu entries, icons and syntax highlighting.
install(FILES
	support/sludge-floormaker.desktop support/sludge-projectmanager.desktop
	support/sludge-spritebankeditor.desktop support/sludge-translationeditor.desktop
	support/sludge-zbuffermaker.desktop
	DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES support/sludge.lang DESTINATION ${CMAKE_INSTALL_DATADIR}/gtksourceview-3.0/language-specs)
install(DIRECTORY support/16x16 support/32x32 support/48x48 support/128x128
	DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor)

# We draw the views offscreen and compare them with the reference images in
# tests/devkit/references. For that, Mesa's EGL is required.
if(SLUDGE_TESTS)
	add_executable(view_test ${PROJECT_SOURCE_DIR}/tests/devkit/view_test.cpp)
	target_compile_definitions(view_test PRIVATE VIEW_TEST_FOLDER="${PROJECT_SOURCE_DIR}/tests/devkit")
	target_link_libraries(view_test PRIVATE devkit-graphics PkgConfig::GTK)
	add_test(NAME devkit_views COMMAND view_test)
endif()
