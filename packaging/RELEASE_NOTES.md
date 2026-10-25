For macOS and Windows, there are two kinds of downloads. **OpenSLUDGE** contains the engine alone, with which you can play any SLUDGE game. **Out Of Order** contains the engine together with Out Of Order, a freeware adventure game made with SLUDGE.

| Download | Platform |
|---|---|
| `OutOfOrder-…-macOS-universal.zip` | Any Mac with Apple Silicon or Intel, from macOS 10.13 on |
| `OutOfOrder-…-macOS-apple-silicon.zip` | Macs with Apple Silicon, from macOS 11 on |
| `OutOfOrder-…-macOS-intel.zip` | Macs with Intel, from macOS 10.13 on |
| `OutOfOrder-…-windows-x64.zip` | 64-bit Windows |
| `OutOfOrder-…-windows-x86.zip` | 32-bit and 64-bit Windows, from Windows XP on, with OpenGL 2.0 graphics |
| `OpenSLUDGE-…-*.zip` | The same platforms, with the engine alone |
| `sludge-….tar.gz` | The source code of the engine, the Dev Kit and the documentation, to build on Linux (see INSTALL) |

If you are not sure which Mac download to take, take the universal one.

Unzip the download and open the app. If macOS does not open it, Control-click the app and choose Open, or click "Open Anyway" in System Settings, under Privacy & Security. If Windows SmartScreen says that it protected your PC, click "More info" and then "Run anyway".

## What's new

This release brings back the SLUDGE Dev Kit, ported to GTK 3, and builds everything with CMake. The engine is the same as in 2.3.0, so for the changes in the engine, see the [release notes of 2.3.0](https://github.com/opensludge/opensludge/releases/tag/2.3.0).

## The Dev Kit

- The Dev Kit, with the project manager, the floor maker, the z-buffer maker, the sprite bank editor and the translation editor, now uses GTK 3 instead of GTK 2 and GtkGLExt, so you can build it again on current Linux distributions. It requires GTK 3.20 or later, libepoxy and libpng.
- The floor maker, the z-buffer maker and the sprite bank editor now draw their views with OpenGL 3.2, and on a HiDPI display, at the full resolution.
- The buttons of the file dialogs are now in your language.
- The project manager now opens images with Loupe by default, instead of Eye of GNOME. You can choose another program in the preferences.
- So far, the Dev Kit is only for Linux, and there is no download of it: build it from the source tarball, as described in INSTALL.

## Building SLUDGE

- CMake now builds the engine, the Dev Kit and the documentation, instead of CMake for the engine and Autotools for the rest. With the options SLUDGE_ENGINE, SLUDGE_DEVKIT and SLUDGE_DOC, you choose what to build. On Linux, CMake builds the Dev Kit by default.
- Each release now comes with a source tarball, `sludge-<version>.tar.gz`.
- With SLUDGE_DOC, we now install all the pictures of the documentation. Some were missing before, for example the background of the headings.
