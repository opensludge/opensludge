#include "sprites.h"

#ifdef __cplusplus
extern "C" {
#endif		

//bool processZBufferData ();
bool saveZBufferFile (const char * name, struct spriteBank *buffers);
bool loadZBufferFile (const char * name, struct spriteBank *loadhere);
void loadZTextures (struct spriteBank *loadhere);

bool loadZBufferFromTGA (const char * fileName, struct spriteBank *loadhere);

// Draw the z-buffers, the selected one in white and the others in purple,
// with the outline of the backdrop and the cut-off line of the selected
// z-buffer.
void drawZBufferView (struct spriteBank * backdrop, int selectedBuffer);

//bool setZBufferClick (int x, int y);

#ifdef __cplusplus
}
#endif
