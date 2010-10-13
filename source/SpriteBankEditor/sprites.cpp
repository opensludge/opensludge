#include <stdlib.h>
#include <string.h>
#include <png.h>

#include "tga.h"
#include "sprites.h"
#include "moreio.h"
#include "messbox.h"

void forgetSpriteBank (spriteBank * forgetme) {
	unsigned int index;
	
	if (forgetme->myPalette.pal)
		delete forgetme->myPalette.pal;
	if (forgetme->myPalette.r)
		delete forgetme->myPalette.r;
	if (forgetme->myPalette.g)
		delete forgetme->myPalette.g;
	if (forgetme->myPalette.b)
		delete forgetme->myPalette.b;
	for (index = 0; index < forgetme->total; index ++) {
		delete forgetme->sprites[index].data;
		glDeleteTextures (1, &forgetme->myPalette.tex_names[index]);
	}
	delete forgetme->sprites;
}

bool reserveSpritePal (spritePalette * sP, int n) {
	if (! sP->pal) {
		sP->pal = new unsigned short int [256];
		sP->r = new unsigned char [256];
		sP->g = new unsigned char [256];
		sP->b = new unsigned char [256];
	}
	sP->total = n;
	return (bool) (sP->pal != NULL) && (sP->r != NULL) && (sP->g != NULL) && (sP->b != NULL);
}

/*
bool savePNGSprite (FILE * fp, struct spriteBank *sprites, int index, bool sig) {
		
	png_structp png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		fclose (fp);
		return false;
	}
	
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr){
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fclose (fp);
		return false;
	}	
	
	png_init_io(png_ptr, fp);
	
	if (!sig) png_set_sig_bytes(png_ptr, 8);
	
	const int h = 21, w = 21;
	
	png_set_IHDR(png_ptr, info_ptr, w, h,
				 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);		
	
	unsigned char * row_pointers[h];

	unsigned char data[w*h*4];
	
	for (int i = 0; i < h; i++) {
		row_pointers[i] = data + 4 * i * w;
		if (!(i % 20) || (i > (h-2))) {
			for (int x = 0; x < w; x++) {
				*(unsigned char *)(row_pointers[i]+x*4) = 255;
				*(unsigned char *)(row_pointers[i]+x*4+1) = 0;
				*(unsigned char *)(row_pointers[i]+x*4+2) = 
				*(unsigned char *)(row_pointers[i]+x*4+3) = 255;
			}			
		} else {
			for (int x = 0; x < w; x++) {
				if (x % 20 && x < (w-1)) {
					*(unsigned char *)(row_pointers[i]+x*4) =
					*(unsigned char *)(row_pointers[i]+x*4+1) =
					*(unsigned char *)(row_pointers[i]+x*4+2) = 0;
					*(unsigned char *)(row_pointers[i]+x*4+3) = 0;
				} else {
					*(unsigned char *)(row_pointers[i]+x*4) = 255;
					*(unsigned char *)(row_pointers[i]+x*4+1) = 0;
					*(unsigned char *)(row_pointers[i]+x*4+2) = 255;
					*(unsigned char *)(row_pointers[i]+x*4+3) = 255;
				}
			}
		}			
	}

	
	png_set_rows(png_ptr, info_ptr, row_pointers);	
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);	
	return true;
}
*/
bool savePNGSprite (FILE * fp, struct spriteBank *sprites, int index, bool sig) {
	
	if (sprites->sprites[index].width < 1) return errorBox ("Error saving", "Can't save a sprite that has no width.");
	if (sprites->sprites[index].height < 1) {
		sprites->sprites[index].height = 1;
		unsigned char * d = new unsigned char [sprites->sprites[index].width*4];
		if (!d) return errorBox ("Error saving", "Out of RAM memory.");
		for (int i = 0; i < sprites->sprites[index].width*4; i++) {
			d[i] = 0;
		}
		delete sprites->sprites[index].data;
		sprites->sprites[index].data = d;
	}
	
	png_structp png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		fclose (fp);
		return false;
	}
	
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr){
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fclose (fp);
		return false;
	}	
	
	png_init_io(png_ptr, fp);
	
	if (!sig) png_set_sig_bytes(png_ptr, 8);
	
	png_set_IHDR(png_ptr, info_ptr, sprites->sprites[index].width, sprites->sprites[index].height,
				 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);		
	
	unsigned char * row_pointers[sprites->sprites[index].height];
	
	for (int i = 0; i < sprites->sprites[index].height; i++) {
		row_pointers[i] = sprites->sprites[index].data + 4*i*sprites->sprites[index].width;
	}
	
	png_set_rows(png_ptr, info_ptr, row_pointers);	
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);	
	return true;
}


bool saveSpriteBank (const char * filename, spriteBank *sprites) {
	int i;
	
	FILE * fp = fopen (filename, "wb");
	if (! fp) {
		return false;
	}
	
	if (sprites->type < 2) {
		// This is version 2 of the DUC format
		put2bytes(0, fp);
		fputc (2, fp);
	} else {
		// This is version 3 of the DUC format - in 32-bit glory!
		put2bytes(0, fp);
		fputc (3, fp);		

		put2bytes (sprites->total, fp);
		
		for (i = 0; i < sprites->total; i ++) {
			putSigned (sprites->sprites[i].xhot, fp);
			putSigned (sprites->sprites[i].yhot, fp);
			if (!savePNGSprite (fp, sprites, i, false)) {
				fclose(fp);
				return false;
			}
		}
		
		fclose(fp);
		return true;
	}
	
	put2bytes (sprites->total, fp);
	
	// Number of colours used, except 0 because that gets added (or ignored) by the engine.
	fputc (sprites->myPalette.total-1, fp); 
	
	for (i = 0; i < sprites->total; i ++) {
		put2bytes (sprites->sprites[i].width, fp);
		put2bytes (sprites->sprites[i].height, fp);
		putSigned (sprites->sprites[i].xhot, fp);
		putSigned (sprites->sprites[i].yhot, fp);
		
		// Run Length compressed data go here
		unsigned char * data = sprites->sprites[i].data;
		unsigned char * lookPointer;
		unsigned int size = sprites->sprites[i].width * sprites->sprites[i].height;

		int lookAhead;
		int x = 0;
		while (x < size) {
			lookPointer = data + 1;
			for (lookAhead = x+1; lookAhead < size; lookAhead++) {
				if (*data + sprites->myPalette.total > 255) break;
				if (lookAhead-x > 255) break;
				if (*data != * lookPointer) break;
				lookPointer++;
			}
			if (lookAhead == x+1) {
				fputc ((* data), fp);
			} else {
				fputc (*data + sprites->myPalette.total, fp);
				fputc (lookAhead - x - 1, fp);
			}
			data = lookPointer;
			x = lookAhead;
		}
	}
				
	for (i = 1; i < sprites->myPalette.total; i ++) {
		fputc (sprites->myPalette.r[i], fp);
		fputc (sprites->myPalette.g[i], fp);
		fputc (sprites->myPalette.b[i], fp);
	}
	
	fclose (fp);
	return true;
}



bool loadSpriteBank (const char * filename, spriteBank *loadhere) {
	int i, total, picwidth, picheight, loadSaveMode = 0, howmany,
		startIndex;
	int totalwidth[256], maxheight[256];
	int numTextures = 0;

	// Open the file	
	FILE * fp = fopen (filename, "rb");
	if (fp == NULL) {
		return errorBox ("Can't open sprite bank", "The file can't be opened. I don't know why.");
	}	
	
	total = get2bytes(fp);
	if (! total) {
		loadSaveMode = fgetc(fp);
		if (loadSaveMode == 1) {
			total = 0;
		} else {
			total = get2bytes(fp);
		}
	}
		
	if (loadSaveMode > 3) return errorBox ("Error opening sprite bank", "Unsupported sprite bank file format");
		if (total <= 0) return errorBox ("Error opening sprite bank", "No sprites in bank or invalid sprite bank file");
			
	if (loadSaveMode == 3) {
		loadhere->type = 2;
		
		loadhere->total = total;
		loadhere->sprites = new sprite [total];		
		for (int index = 0; index < total; index ++) {
			loadhere->sprites[index].xhot = getSigned (fp);
			loadhere->sprites[index].yhot = getSigned (fp);

			png_structp png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			if (!png_ptr) {
				fclose (fp);
				return errorBox ("Can't open PNG file", "Error reading the file.");
			}
			
			png_infop info_ptr = png_create_info_struct(png_ptr);
			if (!info_ptr) {
				png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
				fclose (fp);
				return errorBox ("Can't open PNG file", "Error reading the file.");
			}
			
			png_infop end_info = png_create_info_struct(png_ptr);
			if (!end_info) {
				png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
				fclose (fp);
				return errorBox ("Can't open PNG file", "Error reading the file.");
			}
			png_init_io(png_ptr, fp);		// Tell libpng which file to read
			png_set_sig_bytes(png_ptr, 8);	// No sig
			
			png_read_info(png_ptr, info_ptr);
			
			png_uint_32 width, height;
			int bit_depth, color_type, interlace_type, compression_type, filter_method;
			png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);
			
			int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
			
			unsigned char * row_pointers[height];
			unsigned char * data = new unsigned char [rowbytes*height];
			for (int i = 0; i<height; i++)
				row_pointers[i] = data + i*rowbytes;
			
			png_read_image(png_ptr, (png_byte **) row_pointers);
			png_read_end(png_ptr, NULL);
			png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
						
			loadhere->sprites[index].data = data;
			loadhere->sprites[index].width = width;
			loadhere->sprites[index].height = height;
			
			
		}
		fclose(fp);
			
		return true;
	}

	loadhere->type = 0;
			
	loadhere->total = total;
	loadhere->sprites = new sprite [total];
	//if (! checkNew (loadhere->sprites)) return false;

	if (loadSaveMode) {
		howmany = fgetc(fp);
		startIndex = 1;
	}
	
	totalwidth[0] = maxheight[0] = 0;

	for (i = 0; i < total; i ++) {
		switch (loadSaveMode) {
			case 2:
			picwidth = get2bytes(fp);
			picheight = get2bytes(fp);
			loadhere->sprites[i].xhot = getSigned (fp);
			loadhere->sprites[i].yhot = getSigned (fp);
			break;
			
			default:
			picwidth = (unsigned char) fgetc(fp);
			picheight = (unsigned char) fgetc(fp);
			loadhere->sprites[i].xhot = fgetc(fp);
			loadhere->sprites[i].yhot = fgetc(fp);
			break;
		}
		if (totalwidth[numTextures] + picwidth < 2048) {
			loadhere->sprites[i].tex_x = totalwidth[numTextures];
			totalwidth[numTextures] += (loadhere->sprites[i].width = picwidth);
			if ((loadhere->sprites[i].height = picheight) > maxheight[numTextures]) maxheight[numTextures] = picheight;
		} else {
			numTextures++;
			if (numTextures > 255) return false;//fatal ("Can't open sprite bank / font - it's too big.");
			loadhere->sprites[i].tex_x = 0;
			totalwidth[numTextures] = (loadhere->sprites[i].width = picwidth);
			maxheight[numTextures] = loadhere->sprites[i].height = picheight;
		}
		loadhere->sprites[i].texNum = numTextures;

		loadhere->sprites[i].data = (unsigned char *) new unsigned char [picwidth * (picheight + 1)];
		//if (! checkNew (data)) return false;
		int ooo = picwidth * picheight;
		for (int tt = 0; tt < picwidth; tt ++) {
			loadhere->sprites[i].data[ooo ++] = 0;
		}

		switch (loadSaveMode) {
			case 2:			// RUN LENGTH COMPRESSED DATA
			{
				unsigned size = picwidth * picheight;
				unsigned pip = 0;
				
				while (pip < size) {
					unsigned char col = fgetc(fp);
					int looper;
					
					if (col > howmany) {
						col -= howmany + 1;
						looper = fgetc(fp) + 1;
					} else looper = 1;
					
					while (looper --) {
						loadhere->sprites[i].data[pip ++] = col;
					}
				}
			}
			break;
			
			default:		// RAW DATA
				fread (loadhere->sprites[i].data, picwidth, picheight, fp);
			break;
		}
	}
	numTextures++;

	if (! loadSaveMode) {
		howmany = fgetc(fp);
		startIndex = fgetc(fp);
	}

	if (! reserveSpritePal (&loadhere->myPalette, howmany + startIndex)) return false;

	for (i = 0; i < howmany; i ++) {
		loadhere->myPalette.r[i + startIndex] = (unsigned char) fgetc(fp);
		loadhere->myPalette.g[i + startIndex] = (unsigned char) fgetc(fp);
		loadhere->myPalette.b[i + startIndex] = (unsigned char) fgetc(fp);
		loadhere->myPalette.pal[i + startIndex] = makeColour (loadhere->myPalette.r[i + startIndex], loadhere->myPalette.g[i + startIndex], loadhere->myPalette.b[i + startIndex]);
	}
			
	fclose(fp);
	return true;
}

// I found this function on a coding forum on the 'net.
// Looks a bit weird, but it should work.
int getNextPOT(int n) {
	--n;
	n |= n >> 16;
	n |= n >> 8;
	n |= n >> 4;
	n |= n >> 2;
	n |= n >> 1;
	++n;
	return n;
}

bool NPOT_textures = false;


void NPOT_check() {
	/* Check for graphics capabilities... */
	if (GLEE_VERSION_2_0) {
		// Yes! Textures can be any size!
		NPOT_textures = true;
		fprintf (stderr, "OpenGL 2.0! All is good.\n");
	} else {
		if (GLEE_VERSION_1_5) {
			fprintf (stderr, "OpenGL 1.5!\n");
		}
		else if (GLEE_VERSION_1_4) {
			fprintf (stderr, "OpenGL 1.4!\n");
		}
		else if (GLEE_VERSION_1_3) {
			fprintf (stderr, "OpenGL 1.3!\n");
		}
		else if (GLEE_VERSION_1_2) {
			fprintf (stderr, "OpenGL 1.2!\n");
		}
		if (GLEE_ARB_texture_non_power_of_two) {
			// Yes! Textures can be any size!
			NPOT_textures = true;
		} else {
			// Workaround needed for lesser graphics cards. Let's hope this works...
			NPOT_textures = false;
			fprintf (stderr, "Warning: Old graphics card! GLEE_ARB_texture_non_power_of_two not supported.\n");
		}
	}
}

bool loadSpriteTextures (spriteBank *loadhere) {
	int i;
	int fromhere;
	unsigned char s;

	NPOT_check();
	
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	glGenTextures (loadhere->total, loadhere->myPalette.tex_names);	
	
	if (loadhere->type < 2) {
		GLubyte * tmp;		
		for (i = 0; i < loadhere->total; i ++) {
			loadhere->myPalette.tex_w[i] = loadhere->sprites[i].width;
			loadhere->myPalette.tex_h[i] = loadhere->sprites[i].height;
			loadhere->sprites[i].tex_x = 0.0;
			if (! NPOT_textures) {
				loadhere->myPalette.tex_w[i] = getNextPOT(loadhere->myPalette.tex_w[i]);
				loadhere->myPalette.tex_h[i] = getNextPOT(loadhere->myPalette.tex_h[i]);
			}
			
			int h = loadhere->myPalette.tex_h[i];
			int w = loadhere->myPalette.tex_w[i];
			
			tmp = new GLubyte [w*h*4];
			memset (tmp, 0, w*h*4);
			
			fromhere = 0;
			for (int y = 0; y < loadhere->sprites[i].height; y ++) {
				for (int x = 0; x < loadhere->sprites[i].width; x ++) {
					GLubyte * target = tmp + 4*w*y + x*4;
					s = loadhere->sprites[i].data[fromhere++];
					if (s) {
						target[0] = (GLubyte) loadhere->myPalette.r[s];
						target[1] = (GLubyte) loadhere->myPalette.g[s];
						target[2] = (GLubyte) loadhere->myPalette.b[s];
						target[3] = (GLubyte) 255;
					} else {
						target[0] = (GLubyte) 0;
						target[1] = (GLubyte) 0;
						target[2] = (GLubyte) 0;
						target[3] = (GLubyte) 0;
					}
				}
			}
			
			loadhere->sprites[i].texNum = i;
		
			glBindTexture (GL_TEXTURE_2D, loadhere->myPalette.tex_names[i]);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
			
			delete tmp;		
		}
	} else {
		GLubyte * tmp;
		for (i = 0; i < loadhere->total; i ++) {
			loadhere->myPalette.tex_w[i] = loadhere->sprites[i].width;
			loadhere->myPalette.tex_h[i] = loadhere->sprites[i].height;
			loadhere->sprites[i].tex_x = 0.0;
			if (! NPOT_textures) {
				loadhere->myPalette.tex_w[i] = getNextPOT(loadhere->myPalette.tex_w[i]);
				loadhere->myPalette.tex_h[i] = getNextPOT(loadhere->myPalette.tex_h[i]);
			}
			
			int h = loadhere->myPalette.tex_h[i];
			int w = loadhere->myPalette.tex_w[i];
			
			tmp = new GLubyte [w*h*4];
			memset (tmp, 0, w*h*4);
			
			fromhere = 0;
			for (int y = 0; y < loadhere->sprites[i].height; y ++) {
				for (int x = 0; x < loadhere->sprites[i].width; x ++) {
					GLubyte * target = tmp + 4*w*y + x*4;
					target[0] = loadhere->sprites[i].data[fromhere++];
					target[1] = loadhere->sprites[i].data[fromhere++];
					target[2] = loadhere->sprites[i].data[fromhere++];
					target[3] = loadhere->sprites[i].data[fromhere++];
				}
			}
			
			loadhere->sprites[i].texNum = i;
			glBindTexture (GL_TEXTURE_2D, loadhere->myPalette.tex_names[loadhere->sprites[i].texNum]);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
			
			delete tmp;		
		}
	}
	return true;
}

bool loadSpriteTexture (spriteBank *loadhere, int index) {
	int fromhere;
	unsigned char s;

	NPOT_check();	
	
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);	
	
	if (loadhere->type < 2) {
		loadhere->myPalette.tex_w[index] = loadhere->sprites[index].width;
		loadhere->myPalette.tex_h[index] = loadhere->sprites[index].height;
		loadhere->sprites[index].tex_x = 0.0;
		if (! NPOT_textures) {
			loadhere->myPalette.tex_w[index] = getNextPOT(loadhere->myPalette.tex_w[index]);
			loadhere->myPalette.tex_h[index] = getNextPOT(loadhere->myPalette.tex_h[index]);
		}
		
		int h = loadhere->myPalette.tex_h[index];
		int w = loadhere->myPalette.tex_w[index];
		
		GLubyte *tmp = new GLubyte[w*h*4];		
		memset (tmp, 0, w*h*4);

		fromhere = 0;
		for (int y = 0; y < loadhere->sprites[index].height; y ++) {
			for (int x = 0; x < loadhere->sprites[index].width; x ++) {
				GLubyte * target = tmp + 4*w*y + x*4;
				s = loadhere->sprites[index].data[fromhere++];
				if (s) {
					target[0] = (GLubyte) loadhere->myPalette.r[s];
					target[1] = (GLubyte) loadhere->myPalette.g[s];
					target[2] = (GLubyte) loadhere->myPalette.b[s];
					target[3] = (GLubyte) 255;
				} else {
					target[0] = (GLubyte) 0;
					target[1] = (GLubyte) 0;
					target[2] = (GLubyte) 0;
					target[3] = (GLubyte) 0;
				}
			}
		}
		
		glBindTexture (GL_TEXTURE_2D, loadhere->myPalette.tex_names[loadhere->sprites[index].texNum]);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
		delete[] tmp;
	} else {
		loadhere->myPalette.tex_w[index] = loadhere->sprites[index].width;
		loadhere->myPalette.tex_h[index] = loadhere->sprites[index].height;
		loadhere->sprites[index].tex_x = 0.0;
		if (! NPOT_textures) {
			loadhere->myPalette.tex_w[index] = getNextPOT(loadhere->myPalette.tex_w[index]);
			loadhere->myPalette.tex_h[index] = getNextPOT(loadhere->myPalette.tex_h[index]);
		}
		
		int h = loadhere->myPalette.tex_h[index];
		int w = loadhere->myPalette.tex_w[index];
		
		GLubyte *tmp = new GLubyte[w*h*4];
		memset (tmp, 0, w*h*4);
		
		fromhere = 0;
		for (int y = 0; y < loadhere->sprites[index].height; y ++) {
			for (int x = 0; x < loadhere->sprites[index].width; x ++) {
				GLubyte * target = tmp + 4*w*y + x*4;
				target[0] = loadhere->sprites[index].data[fromhere++];
				target[1] = loadhere->sprites[index].data[fromhere++];
				target[2] = loadhere->sprites[index].data[fromhere++];
				target[3] = loadhere->sprites[index].data[fromhere++];
			}
		}
		
		glBindTexture (GL_TEXTURE_2D, loadhere->myPalette.tex_names[loadhere->sprites[index].texNum]);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
		delete[] tmp;
	}
		
	return true;
}

void addSprite (int i, struct spriteBank *sprites) {
	sprite * newsprites = new sprite [sprites->total+1];
	
	int i1 = sprites->total-1, i2 = sprites->total;
	if (i == sprites->total) {
		newsprites [i2].width = 0;
		newsprites [i2].height = 0;
		newsprites [i2].xhot = 0;
		newsprites [i2].yhot = 0;
		newsprites [i2].tex_x = 0;
		newsprites [i2].texNum = i2;
		newsprites [i2].data = NULL;
		sprites->myPalette.tex_names[i2] = 0;
		sprites->myPalette.tex_w[i2] = 0;
		sprites->myPalette.tex_h[i2] = 0;
		glGenTextures (1, &sprites->myPalette.tex_names[i2]);
		i2--;
	}
	while (i1 >= 0 ) {
		newsprites [i2].width = sprites->sprites[i1].width;
		newsprites [i2].height = sprites->sprites[i1].height;
		newsprites [i2].xhot = sprites->sprites[i1].xhot;
		newsprites [i2].yhot = sprites->sprites[i1].yhot;
		newsprites [i2].tex_x = sprites->sprites[i1].tex_x;
		newsprites [i2].texNum = i2;
		newsprites [i2].data = sprites->sprites[i1].data;
		sprites->myPalette.tex_names[i2] = sprites->myPalette.tex_names[i1];
		sprites->myPalette.tex_w[i2] = sprites->myPalette.tex_w[i1];
		sprites->myPalette.tex_h[i2] = sprites->myPalette.tex_h[i1];
		i2--;
		if (i1 == i) {
			newsprites [i2].width = 0;
			newsprites [i2].height = 0;
			newsprites [i2].xhot = 0;
			newsprites [i2].yhot = 0;
			newsprites [i2].tex_x = 0;
			newsprites [i2].texNum = i2;
			newsprites [i2].data = NULL;
			sprites->myPalette.tex_names[i2] = 0;
			sprites->myPalette.tex_w[i2] = 0;
			sprites->myPalette.tex_h[i2] = 0;
			glGenTextures (1, &sprites->myPalette.tex_names[i2]);
			i2--;
		}
		i1--;
	}
	delete sprites->sprites;
	sprites->sprites = newsprites;
	sprites->total++;
}


void deleteSprite (int i, struct spriteBank *sprites) {
	if (! sprites->total) return;
	if (i >= sprites->total) return;
	
	sprite * newsprites = new sprite [sprites->total-1];
	
	int i1 = 0, i2 = 0;
	while (i1 < sprites->total) {
		if (i1 != i) {
			newsprites [i2].width = sprites->sprites[i1].width;
			newsprites [i2].height = sprites->sprites[i1].height;
			newsprites [i2].xhot = sprites->sprites[i1].xhot;
			newsprites [i2].yhot = sprites->sprites[i1].yhot;
			newsprites [i2].tex_x = sprites->sprites[i1].tex_x;
			newsprites [i2].texNum = i2;//sprites->sprites[i1].texNum;
			newsprites [i2].data = sprites->sprites[i1].data;
			sprites->myPalette.tex_names[i2] = sprites->myPalette.tex_names[i1];
			sprites->myPalette.tex_w[i2] = sprites->myPalette.tex_w[i1];
			sprites->myPalette.tex_h[i2] = sprites->myPalette.tex_h[i1];
			i2++;
		} else {
			if (sprites->sprites[i1].data) {
				delete sprites->sprites[i1].data;
				glDeleteTextures(1, (const GLuint*) &sprites->myPalette.tex_names[i1]);
			}
		}
		i1++;
	}
	delete sprites->sprites;
	sprites->sprites = newsprites;
	sprites->total--;
	
}

unsigned char findClosestPal (spritePalette *searchIn, unsigned char r, unsigned char g, unsigned char b) {
	if (r == 255 && g == 0 && b == 255) return 0;
	
	unsigned char ret = 0;
	int diff = 200000, diff1;
	for (int i=1; i < searchIn->total; i++) {
		diff1 = (searchIn->r[i]-r)*(searchIn->r[i]-r) + (searchIn->g[i]-g)*(searchIn->g[i]-g) + (searchIn->b[i]-b)*(searchIn->b[i]-b);
		if (! diff1) return i;
		if (diff1 < diff) {
			ret = i;
			diff = diff1;
		}
	}
	return ret;
}

int findOrAddPal (spritePalette *searchIn, unsigned char r, unsigned char g, unsigned char b) {
	if (r == 255 && g == 0 && b == 255) return 0;
	
	int i;
	for (i=1; i < searchIn->total; i++) {
		if (!(searchIn->r[i]-r) && !(searchIn->g[i]-g) && !(searchIn->b[i]-b)) return i;
	}
	if (i < 255) {
		searchIn->total++;
		searchIn->r[i]=r;
		searchIn->g[i]=g;
		searchIn->b[i]=b;
		return i;
	}
	return -1;
}


bool loadSpriteFromTGA (const char * file, struct spriteBank *sprites, int index)
{
	palCol thePalette[256];

	// Open the file	
	FILE * fp = fopen (file, "rb");
	if (fp == NULL) {
		errorBox ("Can't open TGA file", "The file can't be opened. I don't know why.");
		return false;
	}
		
	// Grab the header
	TGAHeader imageHeader;
	const char * errorBack;
	errorBack = readTGAHeader (imageHeader, fp, thePalette);
	if (errorBack) {
		fclose (fp);
		errorBox ("Can't open TGA file", errorBack);
		return false;		
	}
	
	unsigned char *data;
		
	if (sprites->type < 2) {
		
		data = new unsigned char[imageHeader.height * imageHeader.width];
		if (! data) {
			fclose (fp);
			errorBox ("Can't create sprite", "Out of memory.");
			return false;		
		}
		
		unsigned char palSize = sprites->myPalette.total;
		unsigned char r,g,b;
		int c;
			
		for (int t2 = imageHeader.height-1; t2>=0; t2 --) {
			for (int t1 = 0; t1 < imageHeader.width; t1 ++) {
				if (! imageHeader.compressed) {
					grabRGB (fp, imageHeader.pixelDepth, r, g, b, thePalette);
				} else {
					grabRGBCompressed (fp, imageHeader.pixelDepth, r, g, b, thePalette);
				}
				if (sprites->type == 1)
					data[t2*imageHeader.width+t1] = findClosestPal (&sprites->myPalette, r, g, b);
				else {
					if ((c = findOrAddPal (&sprites->myPalette, r, g, b)) < 0) {
						fclose(fp);
						errorBox ("Can't create sprite", "The sprite bank palette doesn't have enough room left to add the colours. Change the palette mode to locked or 32-bit and try again.");
						sprites->myPalette.total = palSize;
						delete data;
						return false;
					}
					data[t2*imageHeader.width+t1] = c;
				}
			}
		}
	} else {
		data = new unsigned char[imageHeader.height * imageHeader.width * 4];
		if (! data) {
			fclose (fp);
			errorBox ("Can't create sprite", "Out of memory.");
			return false;		
		}
		
		unsigned char r,g,b,a;
		fprintf (stderr, "Compressed: %d (%d)\n", imageHeader.compressed, imageHeader.pixelDepth);
		
		for (int t2 = imageHeader.height-1; t2>=0; t2 --) {
			for (int t1 = 0; t1 < imageHeader.width; t1 ++) {
				if (! imageHeader.compressed) {
					grabRGBA (fp, imageHeader.pixelDepth, r, g, b, a, thePalette);
				} else {
					grabRGBACompressed (fp, imageHeader.pixelDepth, r, g, b, a, thePalette);
				}

				data[t2*imageHeader.width*4+t1*4] = r;
				data[t2*imageHeader.width*4+t1*4+1] = g;
				data[t2*imageHeader.width*4+t1*4+2] = b;
				data[t2*imageHeader.width*4+t1*4+3] = a;
			}
		}
	}
	fclose (fp);
	if (sprites->sprites[index].data) delete sprites->sprites[index].data;

	sprites->sprites[index].data = data;
	sprites->sprites[index].width = imageHeader.width;
	sprites->sprites[index].height = imageHeader.height;
	
	if (sprites->type>=2) {
	
	int n, r, g, b;
	int width = imageHeader.width;
	int height = imageHeader.height;
	for (int x=0; x<width;x++) {
		for (int y=0; y<height; y++) {
			if (! sprites->sprites[index].data[4*width*y + x*4 + 3]) {
				n = r = g = b = 0;
				if (x>0 && sprites->sprites[index].data[4*width*y + (x-1)*4 + 3]) {
					n++;
					r+= sprites->sprites[index].data[4*width*y + (x-1)*4];
					g+= sprites->sprites[index].data[4*width*y + (x-1)*4+1];
					b+= sprites->sprites[index].data[4*width*y + (x-1)*4+2];
				}
				if (x<width-1 && sprites->sprites[index].data[4*width*y + (x+1)*4 + 3]) {
					n++;
					r+= sprites->sprites[index].data[4*width*y + (x+1)*4];
					g+= sprites->sprites[index].data[4*width*y + (x+1)*4+1];
					b+= sprites->sprites[index].data[4*width*y + (x+1)*4+2];
				}
				if (y>0 && sprites->sprites[index].data[4*width*(y-1) + x*4 + 3]) {
					n++;
					r+= sprites->sprites[index].data[4*width*(y-1) + x*4];
					g+= sprites->sprites[index].data[4*width*(y-1) + x*4+1];
					b+= sprites->sprites[index].data[4*width*(y-1) + x*4+2];
				}
				if (y<height-1 && sprites->sprites[index].data[4*width*(y+1) + x*4 + 3]) {
					n++;
					r+= sprites->sprites[index].data[4*width*(y+1) + x*4];
					g+= sprites->sprites[index].data[4*width*(y+1) + x*4+1];
					b+= sprites->sprites[index].data[4*width*(y+1) + x*4+2];
				}
				if (n) {
					r /= n;
					g /= n;
					b /= n;
					sprites->sprites[index].data[4*width*y + x*4]=r;
					sprites->sprites[index].data[4*width*y + x*4+1]=g;
					sprites->sprites[index].data[4*width*y + x*4+2]=b;
				}
			}
		}
	}	
	}
	
	loadSpriteTexture (sprites, index);
	return true;
}

bool loadSpriteFromPNG (const char * file, struct spriteBank *sprites, int index)
{
	if (sprites->type<2) {
		return errorBox ("Can't open PNG file", "PNG files currently not supported in palette mode. Change to 32-bit mode and try again.");
	}
	
	// Open the file	
	FILE * fp = fopen (file, "rb");
	if (fp == NULL) {
		return errorBox ("Can't open PNG file", "The file can't be opened. I don't know why.");
	}
	
	char tmp[10];
	fread(tmp, 1, 8, fp);
    if (png_sig_cmp((png_byte *) tmp, 0, 8)) {
		fclose (fp);
		return errorBox ("Can't open PNG file", "It doesn't appear to be a valid PNG image.");
    }
	
    png_structp png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
		fclose (fp);
		return errorBox ("Can't open PNG file", "Error reading the file.");
	}
	
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
		fclose (fp);
		return errorBox ("Can't open PNG file", "Error reading the file.");
    }
	
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		fclose (fp);
		return errorBox ("Can't open PNG file", "Error reading the file.");
    }
    png_init_io(png_ptr, fp);		// Tell libpng which file to read
    png_set_sig_bytes(png_ptr, 8);	// 8 bytes already read
	
	png_read_info(png_ptr, info_ptr);
	
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type, compression_type, filter_method;
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);
	
    if (bit_depth < 8) png_set_packing(png_ptr);
	png_set_expand(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);	
	if (bit_depth == 16) png_set_strip_16(png_ptr);

	png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
	
	png_read_update_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);
		
	int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	
	unsigned char * row_pointers[height];
	unsigned char * data = new unsigned char [rowbytes*height];
	for (int i = 0; i<height; i++)
		row_pointers[i] = data + i*rowbytes;
	
	png_read_image(png_ptr, (png_byte **) row_pointers);
	png_read_end(png_ptr, NULL);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

	fclose (fp);
	if (sprites->sprites[index].data) delete sprites->sprites[index].data;
	
	sprites->sprites[index].data = data;
	sprites->sprites[index].width = width;
	sprites->sprites[index].height = height;
	
	int n, r, g, b;
	for (int x=0; x<width;x++) {
		for (int y=0; y<height; y++) {
			if (! sprites->sprites[index].data[4*width*y + x*4 + 3]) {
				n = r = g = b = 0;
				if (x>0 && sprites->sprites[index].data[4*width*y + (x-1)*4 + 3]) {
					n++;
					r+= sprites->sprites[index].data[4*width*y + (x-1)*4];
					g+= sprites->sprites[index].data[4*width*y + (x-1)*4+1];
					b+= sprites->sprites[index].data[4*width*y + (x-1)*4+2];
				}
				if (x<width-1 && sprites->sprites[index].data[4*width*y + (x+1)*4 + 3]) {
					n++;
					r+= sprites->sprites[index].data[4*width*y + (x+1)*4];
					g+= sprites->sprites[index].data[4*width*y + (x+1)*4+1];
					b+= sprites->sprites[index].data[4*width*y + (x+1)*4+2];
				}
				if (y>0 && sprites->sprites[index].data[4*width*(y-1) + x*4 + 3]) {
					n++;
					r+= sprites->sprites[index].data[4*width*(y-1) + x*4];
					g+= sprites->sprites[index].data[4*width*(y-1) + x*4+1];
					b+= sprites->sprites[index].data[4*width*(y-1) + x*4+2];
				}
				if (y<height-1 && sprites->sprites[index].data[4*width*(y+1) + x*4 + 3]) {
					n++;
					r+= sprites->sprites[index].data[4*width*(y+1) + x*4];
					g+= sprites->sprites[index].data[4*width*(y+1) + x*4+1];
					b+= sprites->sprites[index].data[4*width*(y+1) + x*4+2];
				}
				if (n) {
					r /= n;
					g /= n;
					b /= n;
					sprites->sprites[index].data[4*width*y + x*4]=r;
					sprites->sprites[index].data[4*width*y + x*4+1]=g;
					sprites->sprites[index].data[4*width*y + x*4+2]=b;
				}
			}
		}
	}
	
	loadSpriteTexture (sprites, index);
	return true;
}

void doFontification (struct spriteBank *sprites, unsigned int fontifySpaceWidth) {
	
	int which = 1;
	int fromColumn = 0;
	int yhot = (sprites->sprites[0].height * 3) / 4;
	
	for (int thisColumn = 0; thisColumn < sprites->sprites[0].width; thisColumn ++) {
		int y;
		
		// Find out if the column's empty...
		for (y = 0; y < sprites->sprites[0].height; y ++) {
			if (sprites->sprites[0].data[y*sprites->sprites[0].width*4 + thisColumn*4 + 3]) break;
		}
		
		int width = (thisColumn - fromColumn);
		
		// So was it?
		if (y == sprites->sprites[0].height || (thisColumn == sprites->sprites[0].width - 1)) {
			
			// Make sure we didn't find a blank column last time
			if (width) {
				
				// Reserve memory
				GLubyte * toHere = (GLubyte *) new GLubyte [width * sprites->sprites[0].height * 4];
				GLubyte * dataIfOK = toHere;
				if (toHere) {
					
					for (y = 0; y < sprites->sprites[0].height; y ++) {
						GLubyte * from = sprites->sprites[0].data + 4*sprites->sprites[0].width*y + fromColumn*4;
						for (int x = 0; x < width*4; x ++) {
							* (toHere ++) = from[x];
						}
					}
					
					addSprite (which, sprites);
					sprites->sprites[which].width = width;
					sprites->sprites[which].height = sprites->sprites[0].height;
					sprites->sprites[which].yhot = yhot;
					delete sprites->sprites[which].data;
					sprites->sprites[which].data = dataIfOK;
					which ++;
				}					
			}
			fromColumn = thisColumn + 1;
		}
	}
	
	addSprite (which, sprites);
	sprites->sprites[which].width = fontifySpaceWidth;
	sprites->sprites[which].height = 1;
	sprites->sprites[which].yhot = 0;
	delete sprites->sprites[which].data;
	sprites->sprites[which].data = new GLubyte [fontifySpaceWidth * 4];
	memset(sprites->sprites[which].data, 0, fontifySpaceWidth * 4);

	deleteSprite (0, sprites);
	
	loadSpriteTextures (sprites);	
}


bool convertSpriteBank8to32 (struct spriteBank *sprites) {
	if (! sprites->total) return true;

	sprite * newsprites = new sprite [sprites->total];
	if (! newsprites) {
		return errorBox ("Can't convert sprite bank", "Out of memory.");
	}
	
	for (int i = 0; i< sprites->total; i++) {
		unsigned char * data = new unsigned char [sprites->sprites[i].height*sprites->sprites[i].width*4];
		if (! data) {
			while (--i >= 0) {
				delete newsprites[i].data;
			}
			delete newsprites;
			return errorBox ("Can't convert sprite bank", "Out of memory.");
		}
		unsigned char s;
		
		int fromhere = 0;
		for (int y = 0; y < sprites->sprites[i].height; y ++) {
			for (int x = 0; x < sprites->sprites[i].width; x ++) {
				unsigned char * target = data + 4*sprites->sprites[i].width*y + x*4;
				s = sprites->sprites[i].data[fromhere++];
				if (s) {
					target[0] = sprites->myPalette.r[s];
					target[1] = sprites->myPalette.g[s];
					target[2] = sprites->myPalette.b[s];
					target[3] = 255;
				} else {
					target[0] = 0;
					target[1] = 0;
					target[2] = 0;
					target[3] = 0;
				}
			}
		}			
		newsprites[i].data = data;
		
		newsprites [i].width = sprites->sprites[i].width;
		newsprites [i].height = sprites->sprites[i].height;
		newsprites [i].xhot = sprites->sprites[i].xhot;
		newsprites [i].yhot = sprites->sprites[i].yhot;
		newsprites [i].tex_x = sprites->sprites[i].tex_x;
		newsprites [i].texNum = sprites->sprites[i].texNum;		
	}
	
	for (int i = 0; i< sprites->total; i++) {
		delete sprites->sprites[i].data;
	}
	delete sprites->sprites;
	sprites->sprites = newsprites;

	sprites->type = 2;	
	return true;
}

bool exportToPNG (const char * file, struct spriteBank *sprites, int index) {
	if (sprites->type < 2) return errorBox ("Export failed", "Sorry - can only export 32-bit pictures at the moment.");
	
	FILE *fp = fopen(file, "wb");
    if (!fp)
    {
		return false;
    }
		
	int ret = savePNGSprite (fp, sprites, index, true);
	
	fclose(fp);
	return ret;
}

void pasteSprite (sprite * single, const spritePalette *fontPal, bool showBox) {
	
	if (! single) return;
	if (single->texNum < 0) return;
/*	
	float tx1 = 0.0;
	float ty1 = 0.0;
	float tx2 = 1.0;
	float ty2 = 1.0;
 */
	float tx1 = (float)(single->tex_x) / fontPal->tex_w[single->texNum];
	float ty1 = 0.0;
	float tx2 = (float)(single->tex_x + single->width) / fontPal->tex_w[single->texNum];
	float ty2;
	if (single->height<0)
		ty2 = (float)(-(single->height+2))/fontPal->tex_h[single->texNum];
	else
		ty2 = (float)(single->height+2)/fontPal->tex_h[single->texNum];
	
	int x1 = -single->xhot;
	int y1 = -single->yhot;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (showBox) {
		glColor4f(0.35f, 1.0f, 0.35f, 0.5f);
		//    glBegin(GL_LINE_LOOP);
		glBegin(GL_QUADS);			
		{
			glVertex3f(x1, -y1+1, 0.0);
			glVertex3f(x1+single->width, -y1+1, 0.0);
			glVertex3f(x1+single->width, -y1-single->height-1, 0.0);
			glVertex3f(x1, -y1-single->height-1, 0.0);
		}
		glEnd();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	
	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
	
	// Draw the sprite
	glBindTexture (GL_TEXTURE_2D, fontPal->tex_names[single->texNum]);
				
	glBegin(GL_QUADS);			
	glTexCoord2f(tx1, ty1);	glVertex3f(x1, -y1, 0.0);
	glTexCoord2f(tx2, ty1);	glVertex3f(x1+single->width, -y1, 0.0);
	glTexCoord2f(tx2, ty2);	glVertex3f(x1+single->width, -y1-single->height, 0.0);
	glTexCoord2f(tx1, ty2);	glVertex3f(x1, -y1-single->height, 0.0);
	glEnd();
}

