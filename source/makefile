# GNU make makefile for OpenSLUDGE by Tobias Hansen

.PHONY: clean install remove clean_engine clean_DevKit install_engine install_DevKit remove_engine remove_DevKit

all: engine DevKit

clean: clean_engine clean_DevKit

install: install_engine install_DevKit

remove: remove_engine remove_DevKit


engine: Engine CommonCode
	$(MAKE) -C Engine

clean_engine: Engine
	$(MAKE) -C Engine clean

install_engine: Engine
	$(MAKE) -C Engine install

remove_engine: Engine
	$(MAKE) -C Engine remove


DevKit: devkit CommonCode
	$(MAKE) -C devkit

clean_DevKit: devkit
	$(MAKE) -C devkit clean

install_DevKit: devkit
	$(MAKE) -C devkit install

remove_DevKit: devkit
	$(MAKE) -C devkit remove
