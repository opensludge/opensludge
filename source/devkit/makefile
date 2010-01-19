# GNU make makefile for OpenSLUDGE by Tobias Hansen

LIBDIR = `gnustep-config --variable=GNUSTEP_SYSTEM_LIBRARIES`
APPDIR = `gnustep-config --variable=GNUSTEP_SYSTEM_APPS`

DEV_TARGETS = common compiler floormaker projectmanager translator zbuffermaker

.PHONY: clean install remove

all: sludge_devkit

sludge_devkit: $(DEV_TARGETS)
	$(MAKE) -C common
	$(MAKE) -C compiler
	$(MAKE) -C floormaker
	$(MAKE) -C projectmanager
	$(MAKE) -C translator
	$(MAKE) -C zbuffermaker
	ar rcs libsludgedevkit.a common/macstuff.o common/messbox.o common/moreio.o common/splitter.o common/tga.o floormaker/floormaker.o projectmanager/project.o projectmanager/settings.o compiler/allknown.o compiler/checkused.o compiler/compiler.o compiler/dumpfiles.o compiler/floor.o compiler/hsi.o compiler/linker.o compiler/objtype.o compiler/percbar.o compiler/preproc.o compiler/realproc.o compiler/sludge_functions.o compiler/tokens.o compiler/translation.o translator/translator.o zbuffermaker/sprites.o zbuffermaker/zbuffer.o 
	$(MAKE) -C cocoa-gui

clean:
	$(MAKE) -C common clean
	$(MAKE) -C compiler clean
	$(MAKE) -C floormaker clean
	$(MAKE) -C projectmanager clean
	$(MAKE) -C translator clean
	$(MAKE) -C zbuffermaker clean
	rm -f libsludgedevkit.a
	$(MAKE) -C cocoa-gui clean

install: libsludgedevkit.a cocoa-gui/SludgeDevKit.app
	cp -t $(LIBDIR) libsludgedevkit.a
	cp -r -t $(APPDIR) cocoa-gui/SludgeDevKit.app
	
remove:
	rm -f $(LIBDIR)/libsludgedevkit.a
	rm -rf $(APPDIR)/SludgeDevKit.app
