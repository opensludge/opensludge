# GNUstep make makefile for OpenSLUDGE by Tobias Hansen

ifeq ($(GNUSTEP_MAKEFILES),)
 GNUSTEP_MAKEFILES := $(shell gnustep-config --variable=GNUSTEP_MAKEFILES 2>/dev/null)
  ifeq ($(GNUSTEP_MAKEFILES),)
    $(warning )
    $(warning Unable to obtain GNUSTEP_MAKEFILES setting from gnustep-config!)
    $(warning Perhaps gnustep-make is not properly installed,)
    $(warning so gnustep-config is not in your PATH.)
    $(warning )
    $(warning Your PATH is currently $(PATH))
    $(warning )
  endif
endif

include $(GNUSTEP_MAKEFILES)/common.make


APP_NAME=SludgeDevKit

VERSION=104

SludgeDevKit_OBJCC_FILES=\
	AppController.mm\
	FloorDocument.mm\
	ProjectDocument.mm\
	ScriptDocument.mm\
	SpriteBank.mm\
	TranslationDocument.mm\
	zDocument.mm

SludgeDevKit_RESOURCE_FILES=\
	../../../doc/Credits.html\
	../../../doc/SLUDGEDevKitHelp\
	../../../images/Compiler.icns\
	../../../images/ProjIcon.icns\
	../../../images/flags.icns\
	../../../images/floorIcon.icns\
	../../../images/floormodeicon1.png\
	../../../images/floormodeicon2.png\
	../../../images/floormodeicon3.png\
	../../../images/floormodeicon4.png\
	../../../images/floormodeicon5.png\
	../../../images/icon.icns\
	../../../images/slx.icns\
	../../../images/spriteIcon.icns\
	../../../images/zIcon.icns\
	DevKit_Main.nib\
	Floor.nib\
	SludgeDevKitInfo.plist\
	InfoPlist.strings\
	ProjectDocument.nib\
	ScriptDocument.nib\
	SpriteBank.nib\
	TranslationEditor.nib\
	scriptIcon.icns\
	zDocument.nib\
	../libsludgedevkit.a

SludgeDevKit_INCLUDE_DIRS=\
	-I.\
	-I../common\
	-I../compiler\
	-I../floormaker\
	-I../projectmanager\
	-I../translator\
	-I../zbuffermaker\
	-I../../CommonCode\
	-I../../../doc\
	-I../../../images

ADDITIONAL_NATIVE_LIBS+= png
ADDITIONAL_NATIVE_LIBS+= GL
ADDITIONAL_NATIVE_LIBS+= glee

ADDITIONAL_CPPFLAGS+= -DGNUSTEP 

SludgeDevKit_TOOL_LIBS    += -lsludgedevkit
SludgeDevKit_LIB_DIRS     += -L..

include $(GNUSTEP_MAKEFILES)/application.make
