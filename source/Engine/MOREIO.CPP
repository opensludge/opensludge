#include <string.h>
#include <stdint.h>

#include "ALLFILES.H"
#include "MOREIO.H"
#include "NEWFATAL.H"
#include "STRINGY.H"

#include "debug.h"

bool allowAnyFilename = false;


int get2bytes (FILE * fp) {
	int f1, f2;

	f1 = fgetc (fp);
	f2 = fgetc (fp);

	return (f1 * 256 + f2);
}

void put2bytes (int numtoput, FILE * fp) {
	fputc ((char) (numtoput / 256), fp);
	fputc ((char) (numtoput % 256), fp);
}

void writeString (char * s, FILE * fp) {
	int a, len = strlen (s);
	put2bytes (len, fp);
	for (a = 0; a < len; a ++) {
		fputc (s[a] + 1, fp);
	}
}


char * readString (FILE * fp) {

	int a, len = get2bytes (fp);
	//debugOut ("MOREIO: readString - len %i\n", len);
	char * s = new char[len + 1];
	if (! checkNew (s)) {
		return NULL;
	}
	for (a = 0; a < len; a ++) {
		s[a] = (char) (fgetc (fp) - 1);
	}
	s[len] = NULL;
	//debugOut ("MOREIO: readString: %s\n", s);
	return s;
}

float readFloat (FILE * fp) {
	float f;
	fread (& f, sizeof (float), 1, fp);
	return f;
}

void putFloat (float f, FILE * fp) {
	fwrite (& f, sizeof (float), 1, fp);
}

short readSigned (FILE * fp) {
	short f;
	fread (& f, sizeof (short), 1, fp);
	return f;
}

void putSigned (short f, FILE * fp) {
	fwrite (& f, sizeof (short), 1, fp);
}

int32_t get4bytes (FILE * fp) {
/*	int f1, f2, f3, f4;

	f1 = fgetc (fp);
	f2 = fgetc (fp);
	f3 = fgetc (fp);
	f4 = fgetc (fp);

	debugOut ("MOREIO: get4bytes * %i %i %i %i\n", f1, f2, f3, f4);

	return (f1 + f2*256 + f3*256*256 + f4*256*256*256);

*/

	int32_t f;
	fread (& f, sizeof (int32_t), 1, fp);
	return f;
}

void put4bytes (int32_t f, FILE * fp) {
	fwrite (& f, sizeof (int32_t), 1, fp);
}

void debug(char * c, char * c2) {
	FILE * fp = fopen ("argh.txt", "at");
	fprintf (fp, "%s %s\n", c, c2);
	fclose (fp);
}

char * encodeFilename (char * nameIn) {
	if (allowAnyFilename) {
//		deb ("Encoding from:", nameIn);
		char * newName = new char[strlen (nameIn) * 2 + 1];
		int i = 0;
		while (*nameIn) {
			switch (*nameIn) {
				case '<':	newName[i++] = '_';		newName[i++] = 'L';		break;
				case '>':	newName[i++] = '_';		newName[i++] = 'G';		break;
				case '|':	newName[i++] = '_';		newName[i++] = 'P';		break;
				case '_':	newName[i++] = '_';		newName[i++] = 'U';		break;
				case '\"':	newName[i++] = '_';		newName[i++] = 'S';		break;
				case '\\':	newName[i++] = '_';		newName[i++] = 'B';		break;
				case '/':	newName[i++] = '_';		newName[i++] = 'F';		break;
				case ':':	newName[i++] = '_';		newName[i++] = 'C';		break;
				case '*':	newName[i++] = '_';		newName[i++] = 'A';		break;
				case '?':	newName[i++] = '_';		newName[i++] = 'Q';		break;
				
				default:	newName[i++] = *nameIn;							break;
			}
			newName[i] = NULL;
//			deb ("So far:", newName);
			nameIn ++;			
		}
//		deb ("Finished:", newName);
		return newName;
	} else {
		return copyString (nameIn);
	}
}

char * decodeFilename (char * nameIn) {
	if (allowAnyFilename) {
//		deb ("Name in:", nameIn);
		char * newName = new char[strlen (nameIn) + 1];
		int i = 0;
		while (* nameIn) {
			if (* nameIn == '_') {
//				deb ("Special character", nameIn + 1);
				nameIn ++;
				switch (* nameIn) {
					case 'L':	newName[i] = '<';	nameIn ++;		break;
					case 'G':	newName[i] = '>';	nameIn ++;		break;
					case 'P':	newName[i] = '|';	nameIn ++;		break;
					case 'U':	newName[i] = '_';	nameIn ++;		break;
					case 'S':	newName[i] = '\"';	nameIn ++;		break;
					case 'B':	newName[i] = '\\';	nameIn ++;		break;
					case 'F':	newName[i] = '/';	nameIn ++;		break;
					case 'C':	newName[i] = ':';	nameIn ++;		break;
					case 'A':	newName[i] = '*';	nameIn ++;		break;
					case 'Q':	newName[i] = '?';	nameIn ++;		break;
					default:	newName[i] = '_';
				}
			} else {
//				deb ("Normal character", nameIn);
				newName[i] = *nameIn;
				nameIn ++;
			}
			i ++;

//			{
//			newName[i] = NULL;
//			deb ("So far:", newName);
//			}
		}
		newName[i] = NULL;
		return newName;
	} else {
		return copyString (nameIn);
	}
}
