; NSIS2 Script
; by Alexander Shaduri <ashaduri 'at' gmail.com>.
; modified for SLUDGE by Tobias Hansen.
; Compatible with NSIS Unicode 2.45.
; Public Domain

; pass /DNO_GTK to makensis to disable inclusion of gtk.
; (not testet for SLUDGE)

; gtk installer name for embedding
!define GTK_INSTALLER_EXE "gtk2-runtime-2.16.6-2010-05-12-ash.exe"

!define PRODUCT_VERSION "2.2.2"
!define PRODUCT_NAME "SLUDGE"
!define PRODUCT_NAME_SMALL "sludge"
!define PRODUCT_PUBLISHER "SLUDGE Developers"
!define PRODUCT_WEB_SITE "http://opensludge.github.io"

;!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\AppMainExe.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

!define REGISTRY_APP_PATHS "Software\Microsoft\Windows\CurrentVersion\App Paths"


!include "FileFunc.nsh"  ; GetOptions



; --------------- General Settings


; This is needed for proper start menu item manipulation (for all users) in vista
RequestExecutionLevel admin

; This compressor gives us the best results
SetCompressor /SOLID lzma

; Do a CRC check before installing
CRCCheck On

; This is used in titles
Name "${PRODUCT_NAME}"  ;  ${PRODUCT_VERSION}

; Output File Name
!ifdef NO_GTK
	OutFile "${PRODUCT_NAME_SMALL}-${PRODUCT_VERSION}-win-nogtk.exe"
!else
	OutFile "${PRODUCT_NAME_SMALL}-${PRODUCT_VERSION}-win.exe"
!endif


; The Default Installation Directory:
; Try to install to the same directory as runtime.
InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"
; If already installed, try here
InstallDirRegKey HKLM "SOFTWARE\${PRODUCT_NAME}" "InstallationDirectory"


ShowInstDetails show
ShowUnInstDetails show





; --------------------- MUI INTERFACE

; MUI 2.0 compatible install
!include "MUI2.nsh"
!include "InstallOptions.nsh"  ; section description macros

; Backgound Colors. uncomment to enable fullscreen.
; BGGradient 0000FF 000000 FFFFFF

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "images/Sludge.ico"
!define MUI_UNICON "images/Sludge/Sludge_delete.ico"


; Things that need to be extracted on first (keep these lines before any File command!).
; Only useful for BZIP2 compression.
;!insertmacro MUI_RESERVEFILE_LANGDLL
;!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS


; Language Selection Dialog Settings
;!define MUI_LANGDLL_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
;!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
;!define MUI_LANGDLL_REGISTRY_VALUENAME "NSIS:Language"

!define LICENSE_FILE "doc/Credits.rtf"

; Pages to show during installation
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${LICENSE_FILE}"
!ifndef NO_GTK
	!define MUI_PAGE_CUSTOMFUNCTION_LEAVE on_components_page_leave
	!insertmacro MUI_PAGE_COMPONENTS
!endif
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

;!define MUI_FINISHPAGE_RUN "$INSTDIR\gsmartcontrol.exe"
;!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\Example.file"
;!define MUI_FINISHPAGE_RUN_NOTCHECKED
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!insertmacro MUI_PAGE_FINISH



; Uninstaller page
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"


; --------------- END MUI

LangString TEXT_IO_TITLE ${LANG_ENGLISH} "SLUDGE"


var install_option_removeold  ; uninstall the old version first (if present): yes (default), no.



; ----------------- INSTALLATION TYPES

; InstType "Recommended"
; InstType "Full"
; InstType "Minimal"




Section "!SLUDGE" SecMain
SectionIn 1 RO
	SetShellVarContext all  ; use all user variables as opposed to current user
	SetOutPath "$INSTDIR\Dev Kit"
	SetOverwrite On

	File GTK_Dev_Kit/sludge-projectmanager.exe
	File GTK_Dev_Kit/sludge-floormaker.exe
	File GTK_Dev_Kit/sludge-zbuffermaker.exe
	File GTK_Dev_Kit/sludge-spritebankeditor.exe
	File GTK_Dev_Kit/sludge-translationeditor.exe

	File winlibs/libgdkglext-win32-1.0-0.dll
	File winlibs/libgtkglext-win32-1.0-0.dll
	File winlibs/libGLee.dll

	SetOutPath "$INSTDIR\Dev Kit\resources"

	File GTK_Dev_Kit/FloorMaker.glade
	File GTK_Dev_Kit/ZBufferMaker.glade
	File GTK_Dev_Kit/SpriteBankEditor.glade
	File GTK_Dev_Kit/ProjectManager.glade
	File GTK_Dev_Kit/TranslationEditor.glade
	File images/floormodeicon1.png
	File images/floormodeicon2.png
	File images/floormodeicon3.png
	File images/floormodeicon4.png
	File images/floormodeicon5.png
	File images/flags/flags_16x16x32.png
	File images/flags/flags_32x32x32.png
	File images/flags/flags_128x128x32.png
	File images/flags/flags_256x256x32.png
	File images/floorIcon/floorIcon_16x16x32.png
	File images/floorIcon/floorIcon_32x32x32.png
	File images/floorIcon/floorIcon_128x128x32.png
	File images/floorIcon/floorIcon_256x256x32.png
	File images/zIcon/zIcon_16x16x32.png
	File images/zIcon/zIcon_32x32x32.png
	File images/zIcon/zIcon_128x128x32.png
	File images/zIcon/zIcon_256x256x32.png
	File images/spriteIcon/spriteIcon_16x16x32.png
	File images/spriteIcon/spriteIcon_32x32x32.png
	File images/spriteIcon/spriteIcon_128x128x32.png
	File images/spriteIcon/spriteIcon_256x256x32.png
	File images/ProjIcon/ProjIcon_16x16x32.png
	File images/ProjIcon/ProjIcon_32x32x32.png
	File images/ProjIcon/ProjIcon_128x128x32.png
	File images/ProjIcon/ProjIcon_256x256x32.png
	File images/compiler/compiler_16x16x32.png
	File images/gameIcon/gameIcon_16x16x32.png

	SetOutPath "$INSTDIR\icons"
	File images/Sludge.ico
	File images/gameIcon/gameIcon.ico
	File images/scriptIcon/scriptIcon.ico
	File images/slx/slx.ico

	SetOutPath "$INSTDIR\Engine"
	File Engine/ALURE32.dll
	File Engine/color.frag
	File Engine/color.vert
	File Engine/fixScaleSprite.frag
	File Engine/fixScaleSprite.vert
	File Engine/libFLAC.dll
	File Engine/libiconv2.dll
	File Engine/libogg-0.dll
	File Engine/libvorbis-0.dll
	File Engine/libvorbisfile-3.dll
	File Engine/OpenAL32.dll
	File Engine/scale.frag
	File Engine/scale_noaa.frag
	File Engine/scale.vert
	File Engine/SDL.dll
	File "Engine/SLUDGE Engine.exe"
	File Engine/texture.frag
	File Engine/texture.vert
	File Engine/yuv.frag
	File Engine/yuv.vert

	SetOutPath "$INSTDIR"
	File doc/Credits.html
	File "doc/SLUDGE Dev Kit Help.chm"

	; Add Shortcuts (this inherits the exe's run permissions)
	CreateDirectory "$SMPROGRAMS\SLUDGE"
	CreateShortCut "$SMPROGRAMS\SLUDGE\SLUDGE Project Manager.lnk" "$INSTDIR\Dev Kit\sludge-projectmanager.exe" "" \
		"" "" SW_SHOWNORMAL "" "SLUDGE Project Manager - Manage and compile your games"
	CreateShortCut "$SMPROGRAMS\SLUDGE\SLUDGE Floor Maker.lnk" "$INSTDIR\Dev Kit\sludge-floormaker.exe" "" \
		"" "" SW_SHOWNORMAL "" "SLUDGE Floor Maker - Edit SLUDGE floor plans"
	CreateShortCut "$SMPROGRAMS\SLUDGE\SLUDGE Z-Buffer Maker.lnk" "$INSTDIR\Dev Kit\sludge-zbuffermaker.exe" "" \
		"" "" SW_SHOWNORMAL "" "SLUDGE Z-Buffer Maker - Edit SLUDGE z-buffer files"
	CreateShortCut "$SMPROGRAMS\SLUDGE\SLUDGE Sprite Bank Editor.lnk" "$INSTDIR\Dev Kit\sludge-spritebankeditor.exe" "" \
		"" "" SW_SHOWNORMAL "" "SLUDGE Sprite Bank Editor - Edit SLUDGE sprite banks"
	CreateShortCut "$SMPROGRAMS\SLUDGE\SLUDGE Translation Editor.lnk" "$INSTDIR\Dev Kit\sludge-translationeditor.exe" "" \
		"" "" SW_SHOWNORMAL "" "SLUDGE Translation Editor - Edit SLUDGE translation files"
	CreateShortCut "$SMPROGRAMS\SLUDGE\SLUDGE Engine.lnk" "$INSTDIR\Engine\SLUDGE Engine.exe" "" \
		"" "" SW_SHOWNORMAL "" "SLUDGE Engine - Play SLUDGE games"
	CreateShortCut "$SMPROGRAMS\SLUDGE\Help.lnk" "$INSTDIR\SLUDGE Dev Kit Help.chm" "" \
		"" "" SW_SHOWNORMAL "" "Open the SLUDGE help file."
	CreateShortCut "$SMPROGRAMS\SLUDGE\Credits.lnk" "$INSTDIR\Credits.html" "" \
		"" "" SW_SHOWNORMAL "" "Open the credits file, containing licensing information."
	CreateShortCut "$SMPROGRAMS\SLUDGE\Uninstall.lnk" "$INSTDIR\sludge_uninst.exe" "" \
		"" "" SW_SHOWNORMAL "" "Uninstall SLUDGE."
	CreateShortCut "$SMPROGRAMS\SLUDGE\Engine directory.lnk" "$INSTDIR\Engine" "" \
		"" "" SW_SHOWNORMAL "" "Go to Engine directory (copy the files for distribution with your game)."

SectionEnd


Section "Example projects" SecExampleProjects
	SectionIn 1
	SetShellVarContext all  ; use all user variables as opposed to current user

	SetOutPath "$INSTDIR\ExampleProjects\Welcome"
	File doc/ExampleProjects/Welcome/image.tga
	File doc/ExampleProjects/Welcome/script.slu
	File doc/ExampleProjects/Welcome/tada.wav
	File doc/ExampleProjects/Welcome/Welcome.slp

	SetOutPath "$INSTDIR\ExampleProjects\VerbCoin"
	File doc/ExampleProjects/VerbCoin/demofont.duc
	File doc/ExampleProjects/VerbCoin/init.slu
	File doc/ExampleProjects/VerbCoin/VerbCoin.slp
	File doc/ExampleProjects/VerbCoin/german.tra
	File doc/ExampleProjects/VerbCoin/nicupfontsmall.duc

	SetOutPath "$INSTDIR\ExampleProjects\VerbCoin\ego"
	File doc/ExampleProjects/VerbCoin/ego/alien.duc
	File doc/ExampleProjects/VerbCoin/ego/ego.slu

	SetOutPath "$INSTDIR\ExampleProjects\VerbCoin\iface"
	File doc/ExampleProjects/VerbCoin/iface/arrow_icons.slu
	File doc/ExampleProjects/VerbCoin/iface/coin_interface.slu
	File doc/ExampleProjects/VerbCoin/iface/invback.tga
	File doc/ExampleProjects/VerbCoin/iface/inventory.slu
	File doc/ExampleProjects/VerbCoin/iface/keyboard.slu
	File doc/ExampleProjects/VerbCoin/iface/mouse.duc
	File doc/ExampleProjects/VerbCoin/iface/mushroom.duc
	File doc/ExampleProjects/VerbCoin/iface/pop.wav
	File doc/ExampleProjects/VerbCoin/iface/stop_and_go.slu
	File doc/ExampleProjects/VerbCoin/iface/talk.slu
	File doc/ExampleProjects/VerbCoin/iface/verbcoin.tga
	File doc/ExampleProjects/VerbCoin/iface/verbs.slu

	SetOutPath "$INSTDIR\ExampleProjects\VerbCoin\inside"
	File doc/ExampleProjects/VerbCoin/inside/music.xm
	File doc/ExampleProjects/VerbCoin/inside/room.flo
	File doc/ExampleProjects/VerbCoin/inside/room.slu
	File doc/ExampleProjects/VerbCoin/inside/room.tga
	File doc/ExampleProjects/VerbCoin/inside/room.zbu

	SetOutPath "$INSTDIR\ExampleProjects\VerbCoin\outside"
	File doc/ExampleProjects/VerbCoin/outside/duck.duc
	File doc/ExampleProjects/VerbCoin/outside/music.xm
	File doc/ExampleProjects/VerbCoin/outside/room.flo
	File doc/ExampleProjects/VerbCoin/outside/room.slu
	File doc/ExampleProjects/VerbCoin/outside/room.tga
	File doc/ExampleProjects/VerbCoin/outside/room.zbu

	CreateDirectory "$SMPROGRAMS\SLUDGE"
	CreateShortCut "$SMPROGRAMS\SLUDGE\Example projects directory.lnk" "$INSTDIR\ExampleProjects" "" \
		"" "" SW_SHOWNORMAL "" "Browse the code of the example projects."
SectionEnd



!ifndef NO_GTK

; The SecGtkPrivate and SecGtkPublic sections are mutually exclusive.

Section "GTK+ (for this program only)" SecGtkPrivate
	SectionIn 1
	SetShellVarContext all  ; use all user variables as opposed to current user
	AddSize 12200  ; ~ size of unpacked gtk
	SetOutPath "$TEMPDIR"
	File "${GTK_INSTALLER_EXE}"
	; TODO: in the future, when we have translations for this program,
	; make the GTK+ translations installation dependent on their installation status.
	ExecWait '"${GTK_INSTALLER_EXE}" /sideeffects=no /dllpath=root /translations=no /compatdlls=no /S /D=$INSTDIR\Dev Kit'
	Delete "$TEMPDIR\${GTK_INSTALLER_EXE}"
SectionEnd

; disabled by default
Section /o "GTK+ (shared installation)" SecGtkPublic
	SectionIn 1
	SetShellVarContext all  ; use all user variables as opposed to current user
	AddSize 12200  ; ~ size of unpacked gtk
	SetOutPath "$TEMPDIR"
	File "${GTK_INSTALLER_EXE}"
	ExecWait '"${GTK_INSTALLER_EXE}"'
	Delete "$TEMPDIR\${GTK_INSTALLER_EXE}"
SectionEnd

!endif  ; !NO_GTK


!ifndef NO_GTK
var gtk_mode  ; "public", "private" or "none"
var gtk_tmp  ; temporary variable
!endif


; Executed on installation start
Function .onInit
	SetShellVarContext all  ; use all user variables as opposed to current user

	${GetOptions} "$CMDLINE" "/removeold=" $install_option_removeold

	Call PreventMultipleInstances

	Call DetectPrevInstallation

; ask for gtk only if it's a non-gtk installer.
!ifdef NO_GTK
	Call AskForGtk
!endif

!ifndef NO_GTK
	StrCpy $gtk_mode "private" ; default
!endif

FunctionEnd



function .onselchange

!ifndef NO_GTK
	; Remember which gtk section was selected.
	; Deselect the other section.

	; If it was private, we check if public is checked and uncheck private.

	StrCmp $gtk_mode "private" check_public  ; old selection
	StrCmp $gtk_mode "public" check_private  ; old selection
	goto check_exit

	check_public:
		SectionGetFlags ${SecGtkPublic} $gtk_tmp  ; see if it's checked
		IntOp $gtk_tmp $gtk_tmp & ${SF_SELECTED}
		IntCmp $gtk_tmp ${SF_SELECTED} "" check_exit check_exit
		SectionGetFlags ${SecGtkPrivate} $gtk_tmp  ; unselect the other one
		IntOp $gtk_tmp $gtk_tmp & ${SECTION_OFF}
		SectionSetFlags ${SecGtkPrivate} $gtk_tmp
		goto check_exit

	check_private:
		SectionGetFlags ${SecGtkPrivate} $gtk_tmp  ; see if it's checked
		IntOp $gtk_tmp $gtk_tmp & ${SF_SELECTED}
		IntCmp $gtk_tmp ${SF_SELECTED} "" check_exit check_exit
		SectionGetFlags ${SecGtkPublic} $gtk_tmp  ; unselect the other one
		IntOp $gtk_tmp $gtk_tmp & ${SECTION_OFF}
		SectionSetFlags ${SecGtkPublic} $gtk_tmp

	check_exit:


	; store the current mode
	StrCpy $gtk_mode "none"

	SectionGetFlags ${SecGtkPrivate} $gtk_tmp
	IntOp $gtk_tmp $gtk_tmp & ${SF_SELECTED}
	IntCmp $gtk_tmp ${SF_SELECTED} "" mode_end_private mode_end_private
	StrCpy $gtk_mode "private"
	mode_end_private:

	SectionGetFlags ${SecGtkPublic} $gtk_tmp
	IntOp $gtk_tmp $gtk_tmp & ${SF_SELECTED}
	IntCmp $gtk_tmp ${SF_SELECTED} "" mode_end_public mode_end_public
	StrCpy $gtk_mode "public"
	mode_end_public:

	; MessageBox MB_ICONINFORMATION|MB_OK "gtk_mode: $gtk_mode" /SD IDOK
!endif  ; !NO_GTK

functionend


!ifndef NO_GTK
Function on_components_page_leave
	StrCmp $gtk_mode "none" "" noabort
		Call AskForGtk
	noabort:
FunctionEnd
!endif  ; !NO_GTK




; Section descriptions
!ifndef NO_GTK  ; this page is shown only when using gtk
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${SecMain} "SLUDGE - adventure game engine. \
		Development Kit and Engine"
	!insertmacro MUI_DESCRIPTION_TEXT ${SecGtkPrivate} "GTK+ libraries, needed by SLUDGE Dev Kit. \
		This will install a private version of GTK+, usable only by SLUDGE."
	!insertmacro MUI_DESCRIPTION_TEXT ${SecGtkPublic} "GTK+ libraries, needed by SLUDGE Dev Kit. \
		This will install a system-wide version of GTK+, shareable with other programs."
	!insertmacro MUI_DESCRIPTION_TEXT ${SecExampleProjects} "Example projects. \
		This will install example projects (most notably the 'Verb coin example') to the installation directory."
!insertmacro MUI_FUNCTION_DESCRIPTION_END
!endif


; ------------------ POST INSTALL


var gtk_dll_abs_path

Section -post
	SetShellVarContext all  ; use all user variables as opposed to current user

	; Don't set any paths for this exe if it has a private GTK+ installation.
!ifndef NO_GTK
	StrCmp $gtk_mode "private" skip_exe_PATH
!endif
	; set a special path for this exe, as GTK may not be in a global path.
	ReadRegStr $gtk_dll_abs_path HKLM "SOFTWARE\GTK\2.0" "DllPath"
	WriteRegStr HKLM "${REGISTRY_APP_PATHS}\sludge-projectmanager.exe" "Path" "$gtk_dll_abs_path"
	WriteRegStr HKLM "${REGISTRY_APP_PATHS}\sludge-floormaker.exe" "Path" "$gtk_dll_abs_path"
	WriteRegStr HKLM "${REGISTRY_APP_PATHS}\sludge-zbuffermaker.exe" "Path" "$gtk_dll_abs_path"
	WriteRegStr HKLM "${REGISTRY_APP_PATHS}\sludge-spritebankeditor.exe" "Path" "$gtk_dll_abs_path"
	WriteRegStr HKLM "${REGISTRY_APP_PATHS}\sludge-translationeditor.exe" "Path" "$gtk_dll_abs_path"

!ifndef NO_GTK
	skip_exe_PATH:
!endif

!ifndef NO_GTK
	WriteRegStr HKLM "SOFTWARE\${PRODUCT_NAME}" "GtkInstalledMode" "$gtk_mode"
!endif

	WriteRegStr HKLM "SOFTWARE\${PRODUCT_NAME}" "InstallationDirectory" "$INSTDIR"
	WriteRegStr HKLM "SOFTWARE\${PRODUCT_NAME}" "Vendor" "${PRODUCT_PUBLISHER}"

	WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}"
	WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\sludge_uninst.exe"
	WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
	WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
	WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\icons\Sludge.ico"
	WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
	WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
	WriteRegDWORD HKLM "${PRODUCT_UNINST_KEY}" "NoModify" 1
	WriteRegDWORD HKLM "${PRODUCT_UNINST_KEY}" "NoRepair" 1

	WriteRegStr HKCR ".slp" "" "SLUDGE.project"
	WriteRegStr HKCR ".flo" "" "SLUDGE.floor"
	WriteRegStr HKCR ".zbu" "" "SLUDGE.zbuffer"
	WriteRegStr HKCR ".duc" "" "SLUDGE.spritebank"
	WriteRegStr HKCR ".tra" "" "SLUDGE.translation"
	WriteRegStr HKCR ".slg" "" "SLUDGE.game"
	WriteRegStr HKCR ".slu" "" "SLUDGE.script"
	WriteRegStr HKCR ".slu" "PerceivedType" "text"
	WriteRegStr HKCR ".sld" "" "SLUDGE.script"
	WriteRegStr HKCR ".sld" "PerceivedType" "text"
	WriteRegStr HKCR ".slx" "" "SLUDGE.compressedimage"

	WriteRegStr HKCR "SLUDGE.project" "" \
	      "SLUDGE project file"
	WriteRegStr HKCR "SLUDGE.project\DefaultIcon" "" \
	      "$INSTDIR\Dev Kit\sludge-projectmanager.exe,0"
	WriteRegStr HKCR "SLUDGE.project\shell\open\command" "" \
	      '"$INSTDIR\Dev Kit\sludge-projectmanager.exe" "%1"'

	WriteRegStr HKCR "SLUDGE.floor" "" \
	      "SLUDGE floor plan"
	WriteRegStr HKCR "SLUDGE.floor\DefaultIcon" "" \
	      "$INSTDIR\Dev Kit\sludge-floormaker.exe,0"
	WriteRegStr HKCR "SLUDGE.floor\shell\open\command" "" \
	      '"$INSTDIR\Dev Kit\sludge-floormaker.exe" "%1"'

	WriteRegStr HKCR "SLUDGE.zbuffer" "" \
	      "SLUDGE z-buffer file"
	WriteRegStr HKCR "SLUDGE.zbuffer\DefaultIcon" "" \
	      "$INSTDIR\Dev Kit\sludge-zbuffermaker.exe,0"
	WriteRegStr HKCR "SLUDGE.zbuffer\shell\open\command" "" \
	      '"$INSTDIR\Dev Kit\sludge-zbuffermaker.exe" "%1"'

	WriteRegStr HKCR "SLUDGE.spritebank" "" \
	      "SLUDGE sprite bank"
	WriteRegStr HKCR "SLUDGE.spritebank\DefaultIcon" "" \
	      "$INSTDIR\Dev Kit\sludge-spritebankeditor.exe,0"
	WriteRegStr HKCR "SLUDGE.spritebank\shell\open\command" "" \
	      '"$INSTDIR\Dev Kit\sludge-spritebankeditor.exe" "%1"'

	WriteRegStr HKCR "SLUDGE.translation" "" \
	      "SLUDGE translation file"
	WriteRegStr HKCR "SLUDGE.translation\DefaultIcon" "" \
	      "$INSTDIR\Dev Kit\sludge-translationeditor.exe,0"
	WriteRegStr HKCR "SLUDGE.translation\shell\open\command" "" \
	      '"$INSTDIR\Dev Kit\sludge-translationeditor.exe" "%1"'

	WriteRegStr HKCR "SLUDGE.game" "" \
	      "SLUDGE game"
	WriteRegStr HKCR "SLUDGE.game\DefaultIcon" "" \
	      "$INSTDIR\icons\gameIcon.ico"
	WriteRegStr HKCR "SLUDGE.game\shell\open\command" "" \
	      '"$INSTDIR\Engine\SLUDGE Engine.exe" "%1"'

	WriteRegStr HKCR "SLUDGE.script" "" \
	      "SLUDGE script"
	WriteRegStr HKCR "SLUDGE.script\DefaultIcon" "" \
	      "$INSTDIR\icons\scriptIcon.ico"

	WriteRegStr HKCR "SLUDGE.compressedimage" "" \
	      "SLUDGE compressed image"
	WriteRegStr HKCR "SLUDGE.compressedimage\DefaultIcon" "" \
	      "$INSTDIR\icons\slx.ico"

	!insertmacro RefreshShellIconsCall

	; We don't need this, MUI takes care for us
	; WriteRegStr HKCU "Software\${PRODUCT_NAME}" "Installer Language" $sUAGE

	; write out uninstaller
	WriteUninstaller "$INSTDIR\sludge_uninst.exe"

	; uninstall shortcut
	;CreateDirectory "$SMPROGRAMS\GSmartControl"
	;CreateShortCut "$SMPROGRAMS\GSmartControl\Uninstall GSmartControl.lnk" "$INSTDIR\sludge_uninst.exe" "" ""

SectionEnd ; post





; ---------------- UNINSTALL



Function un.onUninstSuccess
	HideWindow
	MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer." /SD IDOK
FunctionEnd




Section Uninstall
	SetShellVarContext all  ; use all user variables as opposed to current user
	SetAutoClose false

!ifndef NO_GTK
	ReadRegStr $gtk_mode HKLM "SOFTWARE\${PRODUCT_NAME}" "GtkInstalledMode"
	StrCmp $gtk_mode "private" "" skip_gtk_remove
		; remove private GTK+, specify the same custom options are during installation
		ExecWait "$INSTDIR\gtk2_runtime_uninst.exe /remove_config=yes /sideeffects=no /dllpath=root /translations=no /compatdlls=no /S"
		; _?=$INSTDIR
		; Delete "$INSTDIR\gtk2_runtime_uninst.exe"  ; If using _? flag, it won't get deleted automatically, do it manually.
	skip_gtk_remove:
!endif

	; add delete commands to delete whatever files/registry keys/etc you installed here.
	Delete "$INSTDIR\sludge_uninst.exe"

	DeleteRegKey HKLM "SOFTWARE\${PRODUCT_NAME}"
	DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"

	DeleteRegKey HKCR "SLUDGE.project"
	DeleteRegKey HKCR "SLUDGE.floor"
	DeleteRegKey HKCR "SLUDGE.zbuffer"
	DeleteRegKey HKCR "SLUDGE.spritebank"
	DeleteRegKey HKCR "SLUDGE.translation"
	DeleteRegKey HKCR "SLUDGE.game"
	DeleteRegKey HKCR "SLUDGE.script"
	DeleteRegKey HKCR "SLUDGE.compressedimage"

	!insertmacro RefreshShellIconsCall

	Delete "$INSTDIR\Credits.html"
	Delete "$INSTDIR\SLUDGE Dev Kit Help.chm"
	RMDir /r "$INSTDIR\icons" 
	RMDir /r "$INSTDIR\Dev Kit"
	RMDir /r "$INSTDIR\Engine"
	RMDir /r "$INSTDIR\ExampleProjects"

	RMDir "$INSTDIR"  ; only if empty

!ifndef NO_GTK
	StrCmp $gtk_mode "private" skip_exe_PATH_remove
!endif

	DeleteRegKey HKLM "${REGISTRY_APP_PATHS}\sludge-projectmanager.exe"
	DeleteRegKey HKLM "${REGISTRY_APP_PATHS}\sludge-floormaker.exe"
	DeleteRegKey HKLM "${REGISTRY_APP_PATHS}\sludge-zbuffermaker.exe"
	DeleteRegKey HKLM "${REGISTRY_APP_PATHS}\sludge-spritebankeditor.exe"
	DeleteRegKey HKLM "${REGISTRY_APP_PATHS}\sludge-translationeditor.exe"

!ifndef NO_GTK
	skip_exe_PATH_remove:
!endif

	Delete "$SMPROGRAMS\SLUDGE\SLUDGE Project Manager.lnk"
	Delete "$SMPROGRAMS\SLUDGE\SLUDGE Floor Maker.lnk"
	Delete "$SMPROGRAMS\SLUDGE\SLUDGE Z-Buffer Maker.lnk"
	Delete "$SMPROGRAMS\SLUDGE\SLUDGE Sprite Bank Editor.lnk"
	Delete "$SMPROGRAMS\SLUDGE\SLUDGE Translation Editor.lnk"
	Delete "$SMPROGRAMS\SLUDGE\SLUDGE Engine.lnk"
	Delete "$SMPROGRAMS\SLUDGE\Help.lnk"
	Delete "$SMPROGRAMS\SLUDGE\Credits.lnk"
	Delete "$SMPROGRAMS\SLUDGE\Uninstall.lnk"
	Delete "$SMPROGRAMS\SLUDGE\Engine directory.lnk"
	Delete "$SMPROGRAMS\SLUDGE\Example projects directory.lnk"
	RMDir "$SMPROGRAMS\SLUDGE"

SectionEnd ; end of uninstall section



; --------------- Helpers

; Detect previous installation
Function DetectPrevInstallation
	; if /removeold=no option is given, don't check anything.
	StrCmp $install_option_removeold "no" old_detect_done

	SetShellVarContext all  ; use all user variables as opposed to current user
	push $R0

	; detect previous installation
	ReadRegStr $R0 HKLM "${PRODUCT_UNINST_KEY}" "UninstallString"
	StrCmp $R0 "" old_detect_done

	MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
		"${PRODUCT_NAME} is already installed. $\n$\nClick `OK` to remove the \
		previous version or `Cancel` to continue anyway." \
		/SD IDOK IDOK old_uninst
		; Abort
		goto old_detect_done

	; Run the old uninstaller
	old_uninst:
		ClearErrors
		IfSilent old_silent_uninst old_nosilent_uninst

		old_nosilent_uninst:
			ExecWait '$R0'
			goto old_uninst_continue

		old_silent_uninst:
			ExecWait '$R0 /S _?=$INSTDIR'

		old_uninst_continue:

		IfErrors old_no_remove_uninstaller

		; You can either use Delete /REBOOTOK in the uninstaller or add some code
		; here to remove to remove the uninstaller. Use a registry key to check
		; whether the user has chosen to uninstall. If you are using an uninstaller
		; components page, make sure all sections are uninstalled.
		old_no_remove_uninstaller:

	old_detect_done: ; old installation not found, all ok

	pop $R0
FunctionEnd



; detect GTK installation (any of available versions)
Function AskForGtk
	SetShellVarContext all  ; use all user variables as opposed to current user
	push $R0

	ReadRegStr $R0 HKLM "SOFTWARE\GTK\2.0" "DllPath"
	StrCmp $R0 "" no_gtk have_gtk

	no_gtk:
		MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
		"GTK2-Runtime is not installed. This product needs it to function properly.$\n\
		Please install GTK2-Runtime from http://gtk-win.sf.net/ first.$\n$\n\
		Click 'Cancel' to abort the installation \
		or 'OK' to continue anyway." \
		/SD IDOK IDOK have_gtk
		;Abort  ; Abort has different meaning from onpage callbacks, so use Quit
		Quit
		goto end_gtk_check

	have_gtk:
		; do nothing

	end_gtk_check:

	pop $R0
FunctionEnd



; Prevent running multiple instances of the installer
Function PreventMultipleInstances
;	Push $R0
;	System::Call 'kernel32::CreateMutexA(i 0, i 0, t ${PRODUCT_NAME}) i .r1 ?e'
;	Pop $R0
;	StrCmp $R0 0 +3
;		MessageBox MB_OK|MB_ICONEXCLAMATION "The installer is already running." /SD IDOK
;		Abort
;	Pop $R0
FunctionEnd



; eof

