#include "allfiles.h"

#include "fileset.h"
#include "sprites.h"
#include "moreio.h"
#include "newfatal.h"
#include "colours.h"
#include "backdrop.h"
#include "sludger.h"
#include "zbuffer.h"
#include "sprites_AA.h"

extern zBufferData zBuffer;
extern inputType input;
extern int cameraX, cameraY;

aaSettingsStruct maxAntiAliasSettings = {true, 100.f, 100.f};

struct drawModeSettings
{
	float alphaMult;
	float colourMult;
	float colourAdd;
};


void aaSave (aaSettingsStruct & aa, FILE * fp)
{
	fputc (aa.useMe, fp);
	putFloat (aa.blurX, fp);
	putFloat (aa.blurY, fp);
}

void aaLoad (aaSettingsStruct & aa, FILE * fp)
{
	aa.useMe = fgetc (fp) ? true : false;
	aa.blurX = readFloat (fp);
	aa.blurY = readFloat (fp);
}

void aaCopy (aaSettingsStruct * toHere, const aaSettingsStruct * fromHere)
{
	toHere->useMe = fromHere->useMe;
	toHere->blurX = fromHere->blurX;
	toHere->blurY = fromHere->blurY;
}

bool aaGetFromStack (aaSettingsStruct * toHere, loadedFunction * fun)
{
	int onOff, x, y;

	if (! getValueType (y, SVT_INT, fun->stack->thisVar)) return false;
	trimStack (fun -> stack);	

	if (! getValueType (x, SVT_INT, fun->stack->thisVar)) return false;
	trimStack (fun -> stack);	

	if (! getValueType (onOff, SVT_INT, fun->stack->thisVar)) return false;
	trimStack (fun -> stack);
	
	toHere->useMe = onOff;
	toHere->blurX = x / 16.f;
	toHere->blurY = y / 16.f;

	return true;
}
