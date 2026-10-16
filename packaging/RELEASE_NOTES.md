For macOS and Windows, there are two kinds of downloads. **OpenSLUDGE** contains the engine alone, with which you can play any SLUDGE game. **Out Of Order** contains the engine together with Out Of Order, a freeware adventure game made with SLUDGE.

| Download | Platform |
|---|---|
| `OutOfOrder-…-macOS-universal.zip` | Any Mac with Apple Silicon or Intel, from macOS 10.13 on |
| `OutOfOrder-…-macOS-apple-silicon.zip` | Macs with Apple Silicon, from macOS 11 on |
| `OutOfOrder-…-macOS-intel.zip` | Macs with Intel, from macOS 10.13 on |
| `OutOfOrder-…-windows-x64.zip` | 64-bit Windows |
| `OutOfOrder-…-windows-x86.zip` | 32-bit and 64-bit Windows, from Windows XP on, with OpenGL 2.0 graphics |
| `OpenSLUDGE-…-*.zip` | The same platforms, with the engine alone |

If you are not sure which Mac download to take, take the universal one.

Unzip the download and open the app. If macOS does not open it, Control-click the app and choose Open, or click "Open Anyway" in System Settings, under Privacy & Security. If Windows SmartScreen says that it protected your PC, click "More info" and then "Run anyway".

## Changes in the engine

- The engine now uses SDL 3 instead of SDL 1.2, and plays sound through SDL instead of OpenAL and Alure.
- There are native builds for Apple Silicon and Intel Macs, and on a Retina display, the engine draws the game at the full resolution.
- There is also a build for 32-bit Windows, which should run on Windows XP and later. We have not tested it on Windows XP itself.
- You can resize the window, and the engine keeps the aspect ratio of the game in a window and in full screen.
- You can type with any keyboard layout.
- The engine now also draws the mouse pointer between game frames, so the pointer moves smoothly even in games with a low frame rate.
- On Windows, the colours of the game are now correct on an HDR display in full screen too, and Windows can apply Auto HDR to the game.
- You can play with a game controller. You move the pointer with the left stick or the d-pad, A and B are the left and right mouse buttons, and with the shoulder buttons, you move the pointer to the things you can click on.
- In the controller settings, you choose an action for each button, including keys such as Space. You open the settings with Alt+C (Option+C on a Mac), or on your controller with Share or Create (PlayStation), View (Xbox), Minus (Nintendo Switch), or Back or Select (other controllers). When you start using a controller, the engine shows how to open them.

## Fixes

- On Linux, the engine now converts the text of old games from Windows-1250 instead of ISO 8859-2, so letters such as Š and Ž are correct.
- The engine no longer crashes when you cancel the file chooser.
- The engine no longer leaks the audio buffers of movies.
