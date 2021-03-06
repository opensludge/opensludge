#include <stdint.h>
#include <string.h>

#include "allfiles.h"
#include "colours.h"
#include "backdrop.h"
#include "graphics.h"
#include "newfatal.h"

extern GLuint snapshotTextureName;
extern unsigned char brightnessLevel;

extern float snapTexW, snapTexH;

unsigned char fadeMode = 2;



//----------------------------------------------------
// PROPER BRIGHTNESS FADING
//----------------------------------------------------

unsigned lastFrom, lastTo;

void transitionFader () {
	glEnable(GL_BLEND);
	
	const GLfloat vertices[] = { 
		0.f, (GLfloat)winHeight, 0.f, 
		(GLfloat)winWidth, (GLfloat)winHeight, 0.f, 
		0.f, 0.f, 0.f,
		(GLfloat)winWidth, 0.f, 0.f
	};

	glUseProgram(shader.color);
	setPMVMatrix(shader.color);
	setPrimaryColor(0.0f, 0.0f, 0.0f, 1.0f - brightnessLevel/255.f);
	drawQuad(shader.color, vertices, 0);

	glUseProgram(0);

	glDisable(GL_BLEND);
}

void transitionCrossFader () {

	if (! snapshotTextureName) return;

	glBindTexture (GL_TEXTURE_2D,snapshotTextureName);
	
	glEnable(GL_BLEND);

	const GLfloat vertices[] = { 
		0.f, (GLfloat)winHeight, 0.f, 
		(GLfloat)winWidth, (GLfloat)winHeight, 0.f, 
		0.f, 0.f, 0.f,
		(GLfloat)winWidth, 0.f, 0.f
	};

	const GLfloat texCoords[] = { 
		0.0f, snapTexH,
		snapTexW, snapTexH,
		0.0f, 0.0f,
		snapTexW, 0.0f
	}; 

	glUseProgram(shader.texture);
	setPMVMatrix(shader.texture);
	glUniform1i(glGetUniformLocation(shader.texture, "modulateColor"), 1);
	setPrimaryColor(1.0f, 1.0f, 1.0f, 1.0f - brightnessLevel/255.f);

	drawQuad(shader.texture, vertices, 1, texCoords);
	glUniform1i(glGetUniformLocation(shader.texture, "modulateColor"), 0);
	glUseProgram(0);

	glDisable(GL_BLEND);
}

void transitionSnapshotBox () {
	
	if (! snapshotTextureName) return; 

	//glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture (GL_TEXTURE_2D,snapshotTextureName);
	
	float xScale = (float) brightnessLevel * winWidth / 510.f;	// 510 = 255*2
	float yScale = (float) brightnessLevel * winHeight / 510.f;
	
	const GLfloat vertices[] = { 
		xScale, winHeight-yScale, 0, 
		winWidth-xScale, winHeight-yScale, 0, 
		xScale, yScale, 0,
		winWidth-xScale, yScale, 0
	};

	const GLfloat texCoords[] = { 
		0.0f, snapTexH,
		snapTexW, snapTexH,
		0.0f, 0.0f,
		snapTexW, 0.0f
	}; 

	glUseProgram(shader.texture);
	setPMVMatrix(shader.texture);

	drawQuad(shader.texture, vertices, 1, texCoords);
	glUseProgram(0);
}

//----------------------------------------------------
// FAST PSEUDO-RANDOM NUMBER STUFF FOR DISOLVE EFFECT
//----------------------------------------------------

#define KK 17
uint32_t randbuffer[KK][2];  // history buffer
int p1, p2;

void resetRandW () {
	int32_t seed = 12345;
	
	for (int i=0; i<KK; i++) {
		for (int j=0; j<2; j++) {
			seed = seed * 2891336453u + 1;
			randbuffer[i][j] = seed;
		}
	}
	
	p1 = 0, p2 = 10;
}

GLubyte * transitionTexture = NULL;
GLuint transitionTextureName = 0;

bool reserveTransitionTexture () {
	
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	
	if (! transitionTexture) {
		transitionTexture = new GLubyte [256*256*4];
		if (! checkNew (transitionTexture)) return false;
	}
	
	if (! transitionTextureName) glGenTextures (1, &transitionTextureName);
	glBindTexture (GL_TEXTURE_2D, transitionTextureName);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	texImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, transitionTexture, transitionTextureName);
	
	return true;
}


void transitionDisolve () {
	
	if (! transitionTextureName) reserveTransitionTexture();

	if (! brightnessLevel) {
		transitionFader ();
		return;
	}
	
	uint32_t n;
	uint32_t y;

	GLubyte * toScreen = transitionTexture;
	GLubyte * end = transitionTexture + (256 * 256*4);
	
	do {
		// generate next number
		n = randbuffer[p1][1];
		y = (n << 27) | ((n >> (32 - 27)) + randbuffer[p2][1]);

		n = randbuffer[p1][0];
		randbuffer[p1][1] = (n << 19) | ((n >> (32 - 19)) + randbuffer[p2][0]);
		randbuffer[p1][0] = y;

		// rotate list pointers
		if (! p1 --) p1 = KK - 1;
		if (! p2 --) p2 = KK - 1;
		
		if ((y & 255u) > brightnessLevel) {
			toScreen[0] =toScreen[1] =toScreen[2] = 0;
			toScreen[3] = 255;
		} else {
			toScreen[0] =toScreen[1] =toScreen[2] =toScreen[3] = 0;
		}
		toScreen += 4;
	} while (toScreen < end);

	texImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, transitionTexture, transitionTextureName);

	glEnable(GL_BLEND);

	const GLfloat vertices[] = { 
		0.f, (GLfloat)winHeight, 0.f, 
		(GLfloat)winWidth, (GLfloat)winHeight, 0.f, 
		0.f, 0.f, 0.f,
		(GLfloat)winWidth, 0.f, 0.f
	};

	const GLfloat texCoords[] = { 
		0.0f, 1.0f,
		1.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f
	}; 

	glUseProgram(shader.texture);
	setPMVMatrix(shader.texture);
	glUniform1i(glGetUniformLocation(shader.texture, "modulateColor"), 1);
	setPrimaryColor(1.0f, 1.0f, 1.0f, 1.0f);

	drawQuad(shader.texture, vertices, 1, texCoords);
	glUniform1i(glGetUniformLocation(shader.texture, "modulateColor"), 0);
	glUseProgram(0);

	glDisable(GL_BLEND);
}

void transitionTV () {

	if (! transitionTextureName) reserveTransitionTexture();

	uint32_t n;
	uint32_t y;

	GLubyte * toScreen = transitionTexture;
	GLubyte * end = transitionTexture + (256 * 256*4);
	
	do {
		// generate next number
		n = randbuffer[p1][1];
		y = (n << 27) | ((n >> (32 - 27)) + randbuffer[p2][1]);

		n = randbuffer[p1][0];
		randbuffer[p1][1] = (n << 19) | ((n >> (32 - 19)) + randbuffer[p2][0]);
		randbuffer[p1][0] = y;

		// rotate list pointers
		if (! p1 --) p1 = KK - 1;
		if (! p2 --) p2 = KK - 1;
		
		if ((y & 255u) > brightnessLevel) {
			toScreen[0] =toScreen[1] =toScreen[2] = (n & 255);
			toScreen[3] = (n & 255);
		} else {
			toScreen[0] =toScreen[1] =toScreen[2] =toScreen[3] = 0;
		}
		toScreen += 4;
	} while (toScreen < end);
	
	texImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, transitionTexture, transitionTextureName);
	
	glEnable(GL_BLEND);
	
	const GLfloat vertices[] = { 
		0.f, (GLfloat)winHeight, 0.f, 
		(GLfloat)winWidth, (GLfloat)winHeight, 0.f, 
		0.f, 0.f, 0.f,
		(GLfloat)winWidth, 0.f, 0.f
	};

	const GLfloat texCoords[] = { 
		0.0f, 1.0f,
		1.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f
	}; 

	glUseProgram(shader.texture);
	setPMVMatrix(shader.texture);
	glUniform1i(glGetUniformLocation(shader.texture, "modulateColor"), 1);
	setPrimaryColor(1.0f, 1.0f, 1.0f, 1.0f);

	drawQuad(shader.texture, vertices, 1, texCoords);
	glUniform1i(glGetUniformLocation(shader.texture, "modulateColor"), 0);
	glUseProgram(0);

	glDisable(GL_BLEND);	
}

void transitionBlinds () {
	if (! transitionTextureName) reserveTransitionTexture();
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	GLubyte * toScreen = transitionTexture;

	int level = brightnessLevel/8;
	
	if (level) memset (toScreen, 0, 256*32*level);
	if (level < 32) memset (toScreen+256*32*level, 255, 256*32*(32-level));
	
	texImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, transitionTexture, transitionTextureName);

	glEnable(GL_BLEND);

	const GLfloat vertices[] = { 
		0.f, (GLfloat)winHeight, 0.f, 
		(GLfloat)winWidth, (GLfloat)winHeight, 0.f, 
		0.f, 0.f, 0.f,
		(GLfloat)winWidth, 0.f, 0.f
	};

	const GLfloat texCoords[] = { 
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 25.0f,
		1.0f, 25.0f
	}; 

	glUseProgram(shader.texture);
	setPMVMatrix(shader.texture);
	glUniform1i(glGetUniformLocation(shader.texture, "modulateColor"), 1);
	setPrimaryColor(0.0f, 0.0f, 0.0f, 1.0f);

	drawQuad(shader.texture, vertices, 1, texCoords);
	glUniform1i(glGetUniformLocation(shader.texture, "modulateColor"), 0);
	glUseProgram(0);

	glDisable(GL_BLEND);
}

//----------------------------------------------------

void fixBrightness () {
	switch (fadeMode) {
		case 0:		transitionFader ();				break;		
		case 1:		resetRandW ();					// Fall through!
		case 2:		transitionDisolve ();			break;
		case 3:		transitionTV ();				break;
		case 4:		transitionBlinds ();			break;
		case 5:		transitionCrossFader ();		break;
		case 6:		transitionSnapshotBox ();		break;
 
	}
}
