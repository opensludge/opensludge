#include "sprites.h"

#ifdef __cplusplus
extern "C" {
#endif		

//bool processZBufferData ();
bool saveZBufferFile (const char * name, struct spriteBank *buffers);
bool loadZBufferFile (const char * name, struct spriteBank *loadhere);
void loadZTextures (struct spriteBank *loadhere);

bool loadZBufferFromTGA (const char * fileName, struct spriteBank *loadhere);

//bool setZBufferClick (int x, int y);

#ifdef __cplusplus
}
#endif
