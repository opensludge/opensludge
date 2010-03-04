#include "ALLFILES.H"

#include "FILESET.H"
#include "sprites.h"
#include "MOREIO.H"
#include "NEWFATAL.H"
#include "COLOURS.H"
#include "BACKDROP.H"
#include "SLUDGER.H"
#include "zbuffer.h"
#include "Sprites_AA.h"

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
	aa.blurX = getFloat (fp);
	aa.blurY = getFloat (fp);
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
