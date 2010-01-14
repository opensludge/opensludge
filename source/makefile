# GNU make makefile for OpenSLUDGE by Tobias Hansen

.PHONY: clean install remove

all: Engine CommonCode
	$(MAKE) -C Engine

clean: Engine
	$(MAKE) -C Engine clean

install: Engine
	$(MAKE) -C Engine install

remove: Engine
	$(MAKE) -C Engine remove
