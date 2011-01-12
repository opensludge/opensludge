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
EXTRA_DIST = GPL debian/devkit/gtksourceview-2.0/language-specs/sludge.lang
