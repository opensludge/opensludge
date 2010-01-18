# GNU make makefile for OpenSLUDGE by Tobias Hansen

.PHONY: clean install remove clean_all clean_DevKit install_all install_DevKit remove_all remove_DevKit

engine: Engine CommonCode
	$(MAKE) -C Engine

clean: Engine
	$(MAKE) -C Engine clean

install: Engine
	$(MAKE) -C Engine install

remove: Engine
	$(MAKE) -C Engine remove


DevKit: devkit CommonCode
	$(MAKE) -C devkit

clean_DevKit: devkit
	$(MAKE) -C devkit clean

install_DevKit: devkit
	$(MAKE) -C devkit install

remove_DevKit: devkit
	$(MAKE) -C devkit remove


all: engine DevKit

clean_all: clean clean_DevKit

install_all: install install_DevKit

remove_all: remove remove_DevKit
