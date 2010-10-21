#!/bin/sh
i586-mingw32msvc-windres FloorMaker.rc -O coff -o FloorMakerResources.o
i586-mingw32msvc-windres SpriteBankEditor.rc -O coff -o SpriteBankEditorResources.o
i586-mingw32msvc-windres ZBufferMaker.rc -O coff -o ZBufferMakerResources.o
i586-mingw32msvc-windres ProjectManager.rc -O coff -o ProjectManagerResources.o
