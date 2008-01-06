struct TGAHeader {
	unsigned char IDBlockSize;
	unsigned char gotMap;
	BOOL compressed;
	unsigned short int firstPalColour;
	unsigned short int numPalColours;
	unsigned char bitsPerPalColour;
	unsigned short xOrigin;
	unsigned short yOrigin;
	unsigned short width;
	unsigned short height;
	unsigned char pixelDepth;
	unsigned char imageDescriptor;
};

struct palCol
{
	byte r,g,b;
};

unsigned short int makeColour (byte r, byte g, byte b);
unsigned short readAColour (FILE * fp, int bpc, palCol thePalette[], int x, int y);
unsigned short readCompressedColour (FILE * fp, int bpc, palCol thePalette[], int x, int y);
char * readTGAHeader (TGAHeader & h, FILE * fp, palCol thePalette[]);
void setDither (int dither);
bool getDither ();

inline unsigned short redValue (unsigned short c) { return (c >> 11) << 3; }
inline unsigned short greenValue (unsigned short c) { return ((c >> 5) & 63) << 2; }
inline unsigned short blueValue (unsigned short c) { return (c & 31) << 3; }
inline int brightness (unsigned short c) {
	return ((int) redValue (c)) + ((int) greenValue (c)) + ((int) blueValue (c) >> 1);
}