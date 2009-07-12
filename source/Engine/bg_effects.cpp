#include <stdlib.h> 
#include <string.h>
#ifdef _MSC_VER
#include <malloc.h>
#endif

#include "allfiles.h"
#include "backdrop.h"
#include "colours.h"
#include "newfatal.h"
#include "moreio.h"

#define DEBUG_MATRIX_EFFECTS	0
#define DEBUG_MATRIX_CREATE		0

//extern unsigned short int * * backDropImage;

#if 0
// Raised
static int s_matrixEffectDivide = 2;
static int s_matrixEffectWidth = 3;
static int s_matrixEffectHeight = 3;
static int s_matrixEffectData[9] = {0,0,0,0,-1,0,0,0,2};
static int s_matrixEffectBase = 0;
#elif 0
// Stay put
static int s_matrixEffectDivide = 1;
static int s_matrixEffectWidth = 3;
static int s_matrixEffectHeight = 3;
static int s_matrixEffectData[9] = {0,0,0,0,1,0,0,0,0};
static int s_matrixEffectBase = 0;
#elif 0
// Brighten
static int s_matrixEffectDivide = 9;
static int s_matrixEffectWidth = 1;
static int s_matrixEffectHeight = 1;
static int s_matrixEffectData[9] = {10};
static int s_matrixEffectBase = 15;
#elif 0
// Raised up/left
static int s_matrixEffectDivide = 4;
static int s_matrixEffectWidth = 3;
static int s_matrixEffectHeight = 3;
static int s_matrixEffectData[9] = {-2,-1,0,-1,1,1,0,1,2};
static int s_matrixEffectBase = 16;
#elif 0
// Standard emboss
static int s_matrixEffectDivide = 2;
static int s_matrixEffectWidth = 3;
static int s_matrixEffectHeight = 3;
static int s_matrixEffectData[9] = {-1,0,0,0,0,0,0,0,1};
static int s_matrixEffectBase = 128;
#elif 0
// Horizontal blur
static int s_matrixEffectDivide = 11;
static int s_matrixEffectWidth = 5;
static int s_matrixEffectHeight = 1;
static int s_matrixEffectData[9] = {1,3,3,3,1};
static int s_matrixEffectBase = 0;
#elif 0
// Double vision
static int s_matrixEffectDivide = 6;
static int s_matrixEffectWidth = 13;
static int s_matrixEffectHeight = 2;
static int s_matrixEffectData[26] = {2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3};
static int s_matrixEffectBase = 0;
#elif 0
// Negative
static int s_matrixEffectDivide = 1;
static int s_matrixEffectWidth = 1;
static int s_matrixEffectHeight = 1;
static int s_matrixEffectData[9] = {-1};
static int s_matrixEffectBase = 255;
#elif 0
// Fog
static int s_matrixEffectDivide = 4;
static int s_matrixEffectWidth = 1;
static int s_matrixEffectHeight = 1;
static int s_matrixEffectData[9] = {3};
static int s_matrixEffectBase = 45;
#elif 0
// Blur
static int s_matrixEffectDivide = 14;
static int s_matrixEffectWidth = 3;
static int s_matrixEffectHeight = 3;
static int s_matrixEffectData[9] = {1,2,1,2,2,2,1,2,1};
static int s_matrixEffectBase = 0;
#else
static int s_matrixEffectDivide = 0;
static int s_matrixEffectWidth = 0;
static int s_matrixEffectHeight = 0;
static int * s_matrixEffectData = NULL;
static int s_matrixEffectBase = 0;
#endif

void blur_saveSettings (FILE * fp)
{
	if (s_matrixEffectData)
	{
		put4bytes (s_matrixEffectDivide, fp);
		put4bytes (s_matrixEffectWidth, fp);
		put4bytes (s_matrixEffectHeight, fp);
		put4bytes (s_matrixEffectBase, fp);
		fwrite (s_matrixEffectData, sizeof (int), s_matrixEffectWidth * s_matrixEffectHeight, fp);
	}
	else
	{
		put4bytes (0, fp);
		put4bytes (0, fp);
		put4bytes (0, fp);
		put4bytes (0, fp);
	}
}

static int * blur_allocateMemoryForEffect ()
{
	free(s_matrixEffectData);
	s_matrixEffectData = NULL;
	
	if (s_matrixEffectWidth && s_matrixEffectHeight)
	{
		s_matrixEffectData = (int *) malloc (sizeof (int) * s_matrixEffectHeight * s_matrixEffectWidth);
		checkNew (s_matrixEffectData);
	}
	return s_matrixEffectData;
}

void blur_loadSettings (FILE * fp)
{
	s_matrixEffectDivide = get4bytes(fp);
	s_matrixEffectWidth = get4bytes(fp);
	s_matrixEffectHeight = get4bytes(fp);
	s_matrixEffectBase = get4bytes(fp);

	if (blur_allocateMemoryForEffect())
	{
		fread (s_matrixEffectData, sizeof (int), s_matrixEffectWidth * s_matrixEffectHeight, fp);
	}
	else
	{
		fseek (fp, sizeof (int) * s_matrixEffectWidth * s_matrixEffectHeight, SEEK_CUR);
	}
}

bool blur_createSettings (int numParams, variableStack * & stack)
{
	bool createNullThing = true;
	const char * error = NULL;

#if DEBUG_MATRIX_CREATE
	FILE * debugFp = fopen ("matrixDebug.txt", "at");
	fprintf (debugFp, "blur_createSettings: %d param(s)\n", numParams);
#endif
		
	if (numParams >= 3)
	{
		// PARAMETERS: base, divide, stack (, stack (, stack...))
	
		int height = numParams - 2;
		int width = 0;

		variableStack * justToCheckSizes = stack;
		for (int a = 0; a < height; a ++)
		{
			if (justToCheckSizes->thisVar.varType != SVT_STACK)
			{
				error = "Third and subsequent parameters in setBackgroundEffect should be arrays";
				break;
			}
			else
			{
				int w = stackSize (justToCheckSizes->thisVar.varData.theStack);
				if (a)
				{
					if (w != width)
					{
						error = "Arrays in setBackgroundEffect must be the same size";
						break;
					}
					if (w < width)
					{
						width = w;
					}
				}
				else
				{
					width = w;
				}
			}
		}
		
		if (width == 0 && ! error)
		{
			error = "Empty arrays found in setBackgroundEffect parameters";
		}
		
		if (! error)
		{
			s_matrixEffectWidth = width;
			s_matrixEffectHeight = height;
		
#if DEBUG_MATRIX_CREATE
			fprintf (debugFp, "No errors so far, stack params seem to be OK!\neffect width = %d, effect height = %d\n", s_matrixEffectWidth, s_matrixEffectHeight);
#endif
			
			if (blur_allocateMemoryForEffect())
			{
				for (int y = height - 1; y >= 0; y --)
				{
					variableStack * eachNumber = stack->thisVar.varData.theStack->first;
					if (! error)
					{
						for (int x = 0; x < width; x ++)
						{
							int arraySlot = x + (y * width);
//							s_matrixEffectData[arraySlot] = (rand() % 4);
							if (!getValueType (s_matrixEffectData[arraySlot], SVT_INT, eachNumber->thisVar))
							{
								error = "";
								break;
							}
							eachNumber=eachNumber->next;
#if DEBUG_MATRIX_CREATE
							fprintf (debugFp, "  Value[%d,%d] = array[%d] = %d\n", x, y, arraySlot, s_matrixEffectData[arraySlot]);
#endif
						}
						trimStack (stack);
					}
				}
				if (! error && !getValueType (s_matrixEffectDivide, SVT_INT, stack -> thisVar))
					error = "";
				trimStack (stack);
				if (! error && !getValueType (s_matrixEffectBase, SVT_INT, stack -> thisVar))
					error = "";
				trimStack (stack);
				if (! error)
				{
#if DEBUG_MATRIX_CREATE
					fprintf (debugFp, "COOL! It worked!\nDivide = %d\nBase = %d\n", s_matrixEffectDivide, s_matrixEffectBase);
#endif
					if (s_matrixEffectDivide)
					{
						createNullThing = false;
					}
					else
					{
						error = "Second parameter of setBackgroundEffect (the 'divide' value) should not be 0!";
					}
				}
			}
			else
			{
				error = "Couldn't allocate memory for effect";
			}
		}
	}
	else
	{
		if (numParams)
		{
			error = "setBackgroundEffect should either have 0 parameters or more than 2";
		}
	}
	
#if DEBUG_MATRIX_CREATE
	fprintf (debugFp, "createNullThing = %d\n", createNullThing);
#endif
		
	if (createNullThing)
	{
		s_matrixEffectDivide = 0;
		s_matrixEffectWidth = 0;
		s_matrixEffectHeight = 0;
		s_matrixEffectBase = 0;
		delete s_matrixEffectData;
		s_matrixEffectData = NULL;
	}

#if DEBUG_MATRIX_CREATE
	fprintf (debugFp, "error = '%s'\n\n", error ? error : "none!");
	fclose (debugFp);
#endif
			
	if (error && error[0])
	{
//		warning (error);
		fatal (error);
	}
	
	return ! createNullThing;
}

static inline int clampi (int i, int min, int max)
{
	return (i >= max) ? max : ((i <= min) ? min : i);
}

static inline void blur_createSourceLine (unsigned short int * createLine, unsigned short int * fromLine, int overlapOnLeft)
{
	int miniX;
	memcpy (createLine + overlapOnLeft, fromLine, sizeof (unsigned short int) * sceneWidth);
	
	for (miniX = 0; miniX < overlapOnLeft; miniX ++)
	{
		createLine[miniX] = fromLine[0];
	}
	
	for (miniX = sceneWidth + overlapOnLeft; miniX < sceneWidth + s_matrixEffectWidth - 1; miniX ++)
	{
		createLine[miniX] = fromLine[sceneWidth - 1];
	}
}

bool blurScreen () {
	/*TODO
	if (s_matrixEffectWidth && s_matrixEffectHeight && s_matrixEffectDivide && s_matrixEffectData)
	{
		unsigned short int * thisLine;
		int y, x;
		bool ok = true;
		int overlapOnLeft = s_matrixEffectWidth / 2;
		int overlapAbove  = s_matrixEffectHeight / 2;
		
		unsigned short int ** sourceLine = new unsigned short int * [s_matrixEffectHeight];
		checkNew (sourceLine);
		if (! sourceLine)
			return false;
	
		for (y = 0; y < s_matrixEffectHeight; y ++)
		{
			sourceLine[y] = new unsigned short int[s_matrixEffectWidth - 1 + sceneWidth];
			ok &= (sourceLine[y] != NULL);
		}
		
	#if DEBUG_MATRIX_EFFECTS
		FILE * debugFp = fopen ("matrixDebug.txt", "at");
		fprintf (debugFp, "matrix dimensions = %d x %d\noverlapOnLeft = %d\noverlapAbove = %d\n",
				 s_matrixEffectWidth, s_matrixEffectHeight, overlapOnLeft, overlapAbove);
	#endif
		
		if (ok)
		{
			for (y = 0; y < s_matrixEffectHeight; y ++)
			{
				int miniY = clampi (y - overlapAbove - 1, 0, sceneHeight - 1);
	
	#if DEBUG_MATRIX_EFFECTS
				fprintf (debugFp, "Initial contents of buffer line %d taken from screen line %d\n",
				 		 y, miniY);
	#endif
				
				blur_createSourceLine (sourceLine[y], backDropImage[miniY], overlapOnLeft);
			}
	
			for (y = 0; y < sceneHeight; y ++) {
				thisLine = backDropImage[y];
				
				//-------------------------
				// Scroll source lines
				//-------------------------
				unsigned short int * tempLine = sourceLine[0];
				for (int miniY = 0; miniY < s_matrixEffectHeight - 1; miniY ++)
				{
					sourceLine[miniY] = sourceLine[miniY + 1];
	#if DEBUG_MATRIX_EFFECTS
					fprintf (debugFp, "Copied sourceLine[%d] over sourceLine[%d]\n", miniY + 1, miniY);
	#endif
				}
				sourceLine[s_matrixEffectHeight - 1] = tempLine;
				{
					int h = s_matrixEffectHeight - 1;
					int miniY = clampi (y + (s_matrixEffectHeight - overlapAbove - 1), 0, sceneHeight - 1);

	#if DEBUG_MATRIX_EFFECTS
					fprintf (debugFp, "Getting sourceLine[%d] from backdrop line %d\n", h, miniY);
	#endif
					blur_createSourceLine (sourceLine[h], backDropImage[miniY], overlapOnLeft);
				}
	#if DEBUG_MATRIX_EFFECTS
				fprintf (debugFp, "Using sourceLine array to blur background line %d\n", y);
	#endif
				for (x = 0; x < sceneWidth; x ++) {
					int totalRed = 0;
					int totalGreen = 0;
					int totalBlue = 0;
					int * matrixElement = s_matrixEffectData;
					for (int miniY = 0; miniY < s_matrixEffectHeight; ++ miniY)
					{
						unsigned short int * pixel = & sourceLine[miniY][x];
						for (int miniX = 0; miniX < s_matrixEffectWidth; ++ miniX)
						{
							
							totalRed 	+= (int)redValue(*pixel) 	* * matrixElement;
							totalGreen 	+= (int)greenValue(*pixel) 	* * matrixElement;
							totalBlue 	+= (int)blueValue(*pixel) 	* * matrixElement;
							++ matrixElement;
							++ pixel;
						}
					}
					totalRed = (totalRed + s_matrixEffectDivide / 2) / s_matrixEffectDivide + s_matrixEffectBase;
					totalRed = (totalRed < 0) ? 0 : ((totalRed > 255) ? 255 : totalRed);
					
					totalGreen = (totalGreen + s_matrixEffectDivide / 2) / s_matrixEffectDivide + s_matrixEffectBase;
					totalGreen = (totalGreen < 0) ? 0 : ((totalGreen > 255) ? 255 : totalGreen);
					
					totalBlue = (totalBlue + s_matrixEffectDivide / 2) / s_matrixEffectDivide + s_matrixEffectBase;
					totalBlue = (totalBlue < 0) ? 0 : ((totalBlue > 255) ? 255 : totalBlue);
					
					* thisLine = (makeColour(totalRed, totalGreen, totalBlue));
					++ thisLine;
				}
			}
		}
		
		for (y = 0; y < s_matrixEffectHeight; y ++)
		{
	#if DEBUG_MATRIX_EFFECTS
			fprintf (debugFp, "delete sourceLine[%d]\n", y);
	#endif
				
			delete sourceLine[y];
		}
		delete sourceLine;
		
	#if DEBUG_MATRIX_EFFECTS
		fprintf (debugFp, "\n");
		fclose (debugFp);
	#endif
	
		return true;
	}
	*/
	return false;
}
