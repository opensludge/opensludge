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
extern unsigned short int * screen;
extern unsigned short int * * backDropImage;
extern inputType input;
extern int cameraX, cameraY;

aaSettingsStruct maxAntiAliasSettings = {false, 100.f, 100.f};

struct drawModeSettings
{
	float alphaMult;
	float colourMult;
	float colourAdd;
};

static drawModeSettings dms[] = {
	/*drawModeNormal,*/			{1.00f, 1.00f, 0.00f},
	/*drawModeTransparent1,*/	{0.25f, 1.00f, 0.00f},
	/*drawModeTransparent2,*/	{0.50f, 1.00f, 0.00f},
	/*drawModeTransparent3,*/	{0.75f, 1.00f, 0.00f},
	/*drawModeDark1,*/			{1.00f, 0.75f, 0.00f},
	/*drawModeDark2,*/			{1.00f, 0.50f, 0.00f},
	/*drawModeDark3,*/			{1.00f, 0.25f, 0.00f},
	/*drawModeBlack,*/			{1.00f, 0.00f, 0.00f},
	/*drawModeShadow1,*/		{0.25f, 0.00f, 0.00f},
	/*drawModeShadow2,*/		{0.50f, 0.00f, 0.00f},
	/*drawModeShadow3,*/		{0.75f, 0.00f, 0.00f},
	/*drawModeFoggy1,*/			{1.00f, 0.75f, 0.125f},
	/*drawModeFoggy2,*/			{1.00f, 0.50f, 0.250f},
	/*drawModeFoggy3,*/			{1.00f, 0.25f, 0.375f},
	/*drawModeFoggy4,*/			{1.00f, 0.00f, 0.500f},
	/*drawModeGlow1,*/			{1.00f, 0.75f, 0.25f},
	/*drawModeGlow2,*/			{1.00f, 0.50f, 0.50f},
	/*drawModeGlow3,*/			{1.00f, 0.25f, 0.75f},
	/*drawModeGlow4,*/			{1.00f, 0.00f, 1.00f},
	/*drawModeInvisible*/		{0.00f, 0.00f, 0.00f},
};

#define numDrawModes (sizeof (dms) / sizeof (drawModeSettings))

BOOL scaleSprite_AA (int xIn, int yIn, sprite & single, const spritePalette & fontPal, float scale, unsigned int drawMode, int floaty, BOOL useZB, unsigned short int * * lightMapImage, bool mirror, bool boundingBoxCollision, aaSettingsStruct * aa) {
	int rightmostPixelToBeAltered, bottommostPixelToBeAltered, leftmostPixelToBeAltered, topmostPixelToBeAltered;	
	int originalY = yIn + cameraY;
	int myLightMapMode = lightMapImage ? lightMapMode : LIGHTMAPMODE_NONE;
	BOOL exactHit = FALSE;

	float blurY = (maxAntiAliasSettings.blurY < aa->blurY ? maxAntiAliasSettings.blurY : aa->blurY) * scale;
	float blurX = (maxAntiAliasSettings.blurX < aa->blurX ? maxAntiAliasSettings.blurX : aa->blurX) * scale;

	if (drawMode >= numDrawModes) drawMode = 0;
	drawModeSettings * settings = & dms[drawMode];
	
	float scaledWidth = single.width * scale;
	float scaledHeight = single.height * scale;
	
	float float_y1 = yIn - (single.yhot - floaty + 0.5f) * scale;
	float float_y2 = float_y1 + scaledHeight;

	int useXHot = mirror ? (single.width - (single.xhot + 1)) : single.xhot;
	float float_x1 = xIn - (useXHot + 0.5f) * scale;
	float float_x2 = float_x1 + scaledWidth;
	
	rightmostPixelToBeAltered = float_x2 + 0.9999999f + blurX;
	bottommostPixelToBeAltered = float_y2 + 0.9999999f + blurY;
	leftmostPixelToBeAltered = float_x1 - blurX;
	topmostPixelToBeAltered = float_y1 - blurY;
	
	if (leftmostPixelToBeAltered < 0)
		leftmostPixelToBeAltered = 0;
	if (topmostPixelToBeAltered < 0)
		topmostPixelToBeAltered = 0;
	if (rightmostPixelToBeAltered >= winWidth)
		rightmostPixelToBeAltered = winWidth - 1;
	if (bottommostPixelToBeAltered >= winHeight)
		bottommostPixelToBeAltered = winHeight - 1;
	
	unsigned short int * lmi = NULL;

	if (myLightMapMode == LIGHTMAPMODE_HOTSPOT)
	{		
		int spritePosX = xIn, spritePosY = yIn;
		if (spritePosX < 0) spritePosX = 0;
		if (spritePosX >= sceneWidth) spritePosX = sceneWidth - 1;
		if (spritePosY < 0) spritePosY = 0;
		if (spritePosY >= sceneHeight) spritePosY = sceneHeight - 1;

		lmi = lightMapImage[spritePosY + cameraY] + spritePosX + cameraX;
	}

	for (int y = topmostPixelToBeAltered; y <= bottommostPixelToBeAltered; y ++)
	{
		const float y1 = (y - 0.5f - float_y1 - blurY) / scale;
		const float y2 = (y + 0.5f - float_y1 + blurY) / scale;
		const int lowY = ((int)(y1 + 1.f)) - 1;
		const int topY = ((int)(y2 + 1.f)) - 1;				
		unsigned short int * toScreen = & screen[winWidth * y + leftmostPixelToBeAltered];

		if (myLightMapMode == LIGHTMAPMODE_PIXEL)
		{
			lmi = lightMapImage[y + cameraY] + leftmostPixelToBeAltered + cameraX;
		}

		for (int x = leftmostPixelToBeAltered; x <= rightmostPixelToBeAltered; x ++)
		{
			if (zBuffer.map && useZB && originalY <= zBuffer.map[y + cameraY][x + cameraX])
			{
			}
			else
			{
				float x1 = (x - 0.5f - float_x1 - blurX) / scale;
				float x2 = (x + 0.5f - float_x1 + blurX) / scale;

				float redTotal = 0;
				float greenTotal = 0;
				float blueTotal = 0;
				float colourDivideByMe = 0;
	
				float alphaTotal = 0;
				float alphaDivideByMe = 0;
				
				const int lowX = ((int)(x1 + 1.f)) - 1;
				const int topX = ((int)(x2 + 1.f)) - 1;
				
				for (int miniY = lowY; miniY <= topY; miniY ++)
				{
					// -- WEIGHTING ------
					float thisColTop = miniY;
					float thisColBot = (miniY + 1);
					if (thisColTop < y1)
						thisColTop = y1;
					if (thisColBot > y2)
						thisColBot = y2;
					float weightingY = (thisColBot - thisColTop);
					// -- WEIGHTING ------
	
					unsigned char * incoming;
					int miniAdder;
	
					if (mirror)
					{
						incoming = & single.data[(single.width - lowX - 1) + miniY * single.width];
						miniAdder = -1;
					}
					else
					{
						incoming = & single.data[lowX + miniY * single.width];
						miniAdder = 1;
					}
					
					for (int miniX = lowX; miniX <= topX; miniX ++)
					{
						// -- WEIGHTING ------
						float thisColLeft = miniX;
						float thisColRight = (miniX + 1);
						if (thisColLeft < x1)
							thisColLeft = x1;
						if (thisColRight > x2)
							thisColRight = x2;
						float weightingX = (thisColRight - thisColLeft);
						// -- WEIGHTING ------
		
						if (miniX < 0 || miniX >= single.width || miniY < 0 || miniY >= single.height)
						{
							// Out of bounds
						}
						else
						{
							unsigned short int s = *incoming;
							
							if (s)
							{
								float red = 0.f, green = 0.f, blue = 0.f, alpha = 0.f;
								unsigned short colour = fontPal.pal[s];
	
								red = ((redValue (colour) + 0.5f) / 255.f);
								green = ((greenValue (colour) + 0.5f) / 255.f);
								blue = ((blueValue (colour) + 0.5f) / 255.f);
								alpha = weightingX * weightingY;
	
								redTotal += red * alpha;
								greenTotal += green * alpha;
								blueTotal += blue * alpha;
								colourDivideByMe += alpha;
								
								alphaTotal += alpha;
							}
						}
						alphaDivideByMe += weightingX * weightingY;
						incoming += miniAdder;
					}
				}
				
				if (colourDivideByMe > 0.f && alphaDivideByMe > 0.f)
				{
					exactHit |= (input.mouseX == x && input.mouseY == y);
	
					alphaTotal *= settings->alphaMult / alphaDivideByMe;
					if (alphaTotal >= 0.f)
					{
						redTotal *= settings->colourMult / colourDivideByMe;
						greenTotal *= settings->colourMult / colourDivideByMe;
						blueTotal *= settings->colourMult / colourDivideByMe;
						if (lmi)
						{
							redTotal *= redValue(*lmi) / 255.f;
							greenTotal *= greenValue(*lmi) / 255.f;
							blueTotal *= blueValue(*lmi) / 255.f;
						}
						redTotal += settings->colourAdd;
						greenTotal += settings->colourAdd;
						blueTotal += settings->colourAdd;
						if (alphaTotal >= 1.f)
						{
							* toScreen = makeColour (redTotal * 255, greenTotal * 255, blueTotal * 255);
	//						screen[winWidth * y + x] = ;
						}
						else
						{
							float oneMinusAlpha = 1.f - alphaTotal;
							alphaTotal *= 255;
							redTotal *= alphaTotal;
							redTotal += redValue(*toScreen) * oneMinusAlpha;
							greenTotal *= alphaTotal;
							greenTotal += greenValue(*toScreen) * oneMinusAlpha;
							blueTotal *= alphaTotal;
							blueTotal += blueValue(*toScreen) * oneMinusAlpha;
							* toScreen = makeColour (redTotal, greenTotal, blueTotal);
						}
					}
				}
	//			else
	//			{
	//				screen[winWidth * y + x] = makeColour (255, 255, 0);
	//			}
			}
			toScreen ++;
			if (myLightMapMode == LIGHTMAPMODE_PIXEL)
			{
				lmi ++;
			}
		}
	}

	return boundingBoxCollision ? (input.mouseX >= leftmostPixelToBeAltered &&
								   input.mouseX <= rightmostPixelToBeAltered &&
								   input.mouseY >= topmostPixelToBeAltered &&
								   input.mouseY <= bottommostPixelToBeAltered) : exactHit;
}

void fixScaleSprite_AA (int xIn, int yIn, sprite & single, const spritePalette & fontPal, float scale, unsigned int drawMode, int floaty, BOOL useZB, unsigned short int * * lightMapImage, const int camX, const int camY, bool mirror, aaSettingsStruct * aa) {
	int rightmostPixelToBeAltered, bottommostPixelToBeAltered, leftmostPixelToBeAltered, topmostPixelToBeAltered;	
	int originalY = yIn + camY;
	int myLightMapMode = lightMapImage ? lightMapMode : LIGHTMAPMODE_NONE;

	// TODO: Use camX and camY when getting light map thingy!
	camX, camY;

	float blurY = (maxAntiAliasSettings.blurY < aa->blurY ? maxAntiAliasSettings.blurY : aa->blurY) * scale;
	float blurX = (maxAntiAliasSettings.blurX < aa->blurX ? maxAntiAliasSettings.blurX : aa->blurX) * scale;

	if (drawMode >= numDrawModes) drawMode = 0;
	drawModeSettings * settings = & dms[drawMode];
	
	float scaledWidth = single.width * scale;
	float scaledHeight = single.height * scale;
	
	float float_y1 = yIn - (single.yhot - floaty + 0.5f) * scale;
	float float_y2 = float_y1 + scaledHeight;

	int useXHot = mirror ? (single.width - (single.xhot + 1)) : single.xhot;
	float float_x1 = xIn - (useXHot + 0.5f) * scale;
	float float_x2 = float_x1 + scaledWidth;
	
	rightmostPixelToBeAltered = float_x2 + 0.9999999f + blurX;
	bottommostPixelToBeAltered = float_y2 + 0.9999999f + blurY;
	leftmostPixelToBeAltered = float_x1 - blurX;
	topmostPixelToBeAltered = float_y1 - blurY;
	
	if (leftmostPixelToBeAltered < 0)
		leftmostPixelToBeAltered = 0;
	if (topmostPixelToBeAltered < 0)
		topmostPixelToBeAltered = 0;
	if (rightmostPixelToBeAltered >= sceneWidth)
		rightmostPixelToBeAltered = sceneWidth - 1;
	if (bottommostPixelToBeAltered >= sceneHeight)
		bottommostPixelToBeAltered = sceneHeight - 1;
	
	unsigned short int * lmi = NULL;

	if (myLightMapMode == LIGHTMAPMODE_HOTSPOT)
	{		
		int spritePosX = xIn, spritePosY = yIn;
		if (spritePosX < 0) spritePosX = 0;
		if (spritePosX >= sceneWidth) spritePosX = sceneWidth - 1;
		if (spritePosY < 0) spritePosY = 0;
		if (spritePosY >= sceneHeight) spritePosY = sceneHeight - 1;

		lmi = lightMapImage[spritePosY + camY] + spritePosX + camX;
	}

	for (int y = topmostPixelToBeAltered; y <= bottommostPixelToBeAltered; y ++)
	{
		const float y1 = (y - 0.5f - float_y1 - blurY) / scale;
		const float y2 = (y + 0.5f - float_y1 + blurY) / scale;
		const int lowY = ((int)(y1 + 1.f)) - 1;
		const int topY = ((int)(y2 + 1.f)) - 1;				
		unsigned short int * toScreen = backDropImage[y] + leftmostPixelToBeAltered;

		if (myLightMapMode == LIGHTMAPMODE_PIXEL)
		{
			lmi = lightMapImage[y + camY] + leftmostPixelToBeAltered + camX;
		}

		for (int x = leftmostPixelToBeAltered; x <= rightmostPixelToBeAltered; x ++)
		{
			if (zBuffer.map && useZB && originalY <= zBuffer.map[y + camY][x + camX])
			{
			}
			else
			{
				float x1 = (x - 0.5f - float_x1 - blurX) / scale;
				float x2 = (x + 0.5f - float_x1 + blurX) / scale;

				float redTotal = 0;
				float greenTotal = 0;
				float blueTotal = 0;
				float colourDivideByMe = 0;
	
				float alphaTotal = 0;
				float alphaDivideByMe = 0;
				
				const int lowX = ((int)(x1 + 1.f)) - 1;
				const int topX = ((int)(x2 + 1.f)) - 1;
				
				for (int miniY = lowY; miniY <= topY; miniY ++)
				{
					// -- WEIGHTING ------
					float thisColTop = miniY;
					float thisColBot = (miniY + 1);
					if (thisColTop < y1)
						thisColTop = y1;
					if (thisColBot > y2)
						thisColBot = y2;
					float weightingY = (thisColBot - thisColTop);
					// -- WEIGHTING ------
	
					unsigned char * incoming;
					int miniAdder;
	
					if (mirror)
					{
						incoming = & single.data[(single.width - lowX - 1) + miniY * single.width];
						miniAdder = -1;
					}
					else
					{
						incoming = & single.data[lowX + miniY * single.width];
						miniAdder = 1;
					}
					
					for (int miniX = lowX; miniX <= topX; miniX ++)
					{
						// -- WEIGHTING ------
						float thisColLeft = miniX;
						float thisColRight = (miniX + 1);
						if (thisColLeft < x1)
							thisColLeft = x1;
						if (thisColRight > x2)
							thisColRight = x2;
						float weightingX = (thisColRight - thisColLeft);
						// -- WEIGHTING ------
		
						if (miniX < 0 || miniX >= single.width || miniY < 0 || miniY >= single.height)
						{
							// Out of bounds
						}
						else
						{
							unsigned short int s = *incoming;
							
							if (s)
							{
								float red = 0.f, green = 0.f, blue = 0.f, alpha = 0.f;
								unsigned short colour = fontPal.pal[s];
	
								red = ((redValue (colour) + 0.5f) / 255.f);
								green = ((greenValue (colour) + 0.5f) / 255.f);
								blue = ((blueValue (colour) + 0.5f) / 255.f);
								alpha = weightingX * weightingY;
	
								redTotal += red * alpha;
								greenTotal += green * alpha;
								blueTotal += blue * alpha;
								colourDivideByMe += alpha;
								
								alphaTotal += alpha;
							}
						}
						alphaDivideByMe += weightingX * weightingY;
						incoming += miniAdder;
					}
				}
				
				if (colourDivideByMe > 0.f && alphaDivideByMe > 0.f)
				{
					alphaTotal *= settings->alphaMult / alphaDivideByMe;
					if (alphaTotal >= 0.f)
					{
						redTotal *= settings->colourMult / colourDivideByMe;
						greenTotal *= settings->colourMult / colourDivideByMe;
						blueTotal *= settings->colourMult / colourDivideByMe;
						if (lmi)
						{
							redTotal *= redValue(*lmi) / 255.f;
							greenTotal *= greenValue(*lmi) / 255.f;
							blueTotal *= blueValue(*lmi) / 255.f;
						}
						redTotal += settings->colourAdd;
						greenTotal += settings->colourAdd;
						blueTotal += settings->colourAdd;
						if (alphaTotal >= 1.f)
						{
							* toScreen = makeColour (redTotal * 255, greenTotal * 255, blueTotal * 255);
	//						screen[winWidth * y + x] = ;
						}
						else
						{
							float oneMinusAlpha = 1.f - alphaTotal;
							alphaTotal *= 255;
							redTotal *= alphaTotal;
							redTotal += redValue(*toScreen) * oneMinusAlpha;
							greenTotal *= alphaTotal;
							greenTotal += greenValue(*toScreen) * oneMinusAlpha;
							blueTotal *= alphaTotal;
							blueTotal += blueValue(*toScreen) * oneMinusAlpha;
							* toScreen = makeColour (redTotal, greenTotal, blueTotal);
						}
					}
				}
	//			else
	//			{
	//				screen[winWidth * y + x] = makeColour (255, 255, 0);
	//			}
			}
			toScreen ++;
			if (myLightMapMode == LIGHTMAPMODE_PIXEL)
			{
				lmi ++;
			}
		}
	}
}

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
