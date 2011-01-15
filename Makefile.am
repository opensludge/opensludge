if COND_ENGINE
  MAYBE_SOURCE = source
endif

if COND_DEVKIT
  MAYBE_GTK_DEV_KIT = GTK_Dev_Kit
endif

if COND_DOC
  MAYBE_DOC = doc
endif

SUBDIRS = $(MAYBE_SOURCE) $(MAYBE_GTK_DEV_KIT) $(MAYBE_DOC)
dist_doc_DATA = AUTHORS README INSTALL
EXTRA_DIST = \
	GPL \
	debian/changelog \
	debian/compat \
	debian/control \
	debian/copyright \
	debian/devkit/pixmaps/sludge-floormaker.xpm \
	debian/devkit/pixmaps/sludge-projectmanager.xpm \
	debian/devkit/pixmaps/sludge-spritebankeditor.xpm \
	debian/devkit/pixmaps/sludge-translationeditor.xpm \
	debian/devkit/pixmaps/sludge-zbuffermaker.xpm \
	debian/devkit/pixmaps/sludge-floormaker16.xpm \
	debian/devkit/pixmaps/sludge-projectmanager16.xpm \
	debian/devkit/pixmaps/sludge-spritebankeditor16.xpm \
	debian/devkit/pixmaps/sludge-translationeditor16.xpm \
	debian/devkit/pixmaps/sludge-zbuffermaker16.xpm \
	debian/engine/applications/sludge-engine.desktop \
	debian/engine/icons/hicolor/128x128/apps/sludge-engine.png \
	debian/engine/icons/hicolor/128x128/mimetypes/application-x-sludge-game.png \
	debian/engine/icons/hicolor/16x16/apps/sludge-engine.png \
	debian/engine/icons/hicolor/16x16/mimetypes/application-x-sludge-game.png \
	debian/engine/icons/hicolor/32x32/apps/sludge-engine.png \
	debian/engine/icons/hicolor/32x32/mimetypes/application-x-sludge-game.png \
	debian/engine/icons/hicolor/48x48/apps/sludge-engine.png \
	debian/engine/icons/hicolor/48x48/mimetypes/application-x-sludge-game.png \
	debian/rules \
	debian/sludge-devkit.1.xml \
	debian/sludge-devkit.install \
	debian/sludge-devkit.manpages \
	debian/sludge-devkit.menu \
	debian/sludge-devkit.sharedmimeinfo \
	debian/sludge-doc.doc-base \
	debian/sludge-doc.install \
	debian/sludge-engine.1.xml \
	debian/sludge-engine.install \
	debian/sludge-engine.manpages \
	debian/sludge-engine.sharedmimeinfo
