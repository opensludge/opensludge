/*
 * draw2d.cpp - Part of the SLUDGE Dev Kit
 */

#include <stdio.h>

#include "draw2d.h"

static const char * vertexShaderSource =
	"#version 150\n"
	"uniform mat4 projection;\n"
	"in vec2 position;\n"
	"in vec2 textureCoordinates;\n"
	"out vec2 fragmentTextureCoordinates;\n"
	"void main () {\n"
	"	fragmentTextureCoordinates = textureCoordinates;\n"
	"	gl_Position = projection * vec4 (position, 0.0, 1.0);\n"
	"}\n";

static const char * fragmentShaderSource =
	"#version 150\n"
	"uniform vec4 colour;\n"
	"uniform bool textured;\n"
	"uniform sampler2D picture;\n"
	"in vec2 fragmentTextureCoordinates;\n"
	"out vec4 fragmentColour;\n"
	"void main () {\n"
	"	fragmentColour = textured ? colour * texture (picture, fragmentTextureCoordinates) : colour;\n"
	"}\n";

static GLuint program = 0, vertexArray, vertexBuffer;
static GLint projectionLocation, colourLocation, texturedLocation;

static GLuint compileShader (GLenum type, const char * source) {
	GLuint shader = glCreateShader (type);
	glShaderSource (shader, 1, &source, NULL);
	glCompileShader (shader);
	GLint compiled;
	glGetShaderiv (shader, GL_COMPILE_STATUS, &compiled);
	if (! compiled) {
		char log[1024];
		glGetShaderInfoLog (shader, sizeof (log), NULL, log);
		fprintf (stderr, "We can't compile a shader: %s\n", log);
	}
	return shader;
}

bool startDrawing2D () {
	if (program) return true;

	GLuint vertexShader = compileShader (GL_VERTEX_SHADER, vertexShaderSource);
	GLuint fragmentShader = compileShader (GL_FRAGMENT_SHADER, fragmentShaderSource);
	program = glCreateProgram ();
	glAttachShader (program, vertexShader);
	glAttachShader (program, fragmentShader);
	glBindAttribLocation (program, 0, "position");
	glBindAttribLocation (program, 1, "textureCoordinates");
	glLinkProgram (program);
	glDeleteShader (vertexShader);
	glDeleteShader (fragmentShader);
	GLint linked;
	glGetProgramiv (program, GL_LINK_STATUS, &linked);
	if (! linked) {
		char log[1024];
		glGetProgramInfoLog (program, sizeof (log), NULL, log);
		fprintf (stderr, "We can't link the shader program: %s\n", log);
		glDeleteProgram (program);
		program = 0;
		return false;
	}
	projectionLocation = glGetUniformLocation (program, "projection");
	colourLocation = glGetUniformLocation (program, "colour");
	texturedLocation = glGetUniformLocation (program, "textured");

	// Each vertex is x, y and the texture coordinates.
	glGenVertexArrays (1, &vertexArray);
	glBindVertexArray (vertexArray);
	glGenBuffers (1, &vertexBuffer);
	glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof (float), (void *) 0);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof (float), (void *) (2 * sizeof (float)));
	glEnableVertexAttribArray (0);
	glEnableVertexAttribArray (1);

	glUseProgram (program);
	glUniform4f (colourLocation, 1.0f, 1.0f, 1.0f, 1.0f);
	return true;
}

// The projection of glOrtho, with the near plane at 1 and the far plane
// at -1.
static void setProjection (float left, float right, float bottom, float top) {
	GLfloat m[16] = {
		2.0f / (right - left), 0, 0, 0,
		0, 2.0f / (top - bottom), 0, 0,
		0, 0, 1.0f, 0,
		-(right + left) / (right - left), -(top + bottom) / (top - bottom), 0, 1.0f
	};
	glUseProgram (program);
	glUniformMatrix4fv (projectionLocation, 1, GL_FALSE, m);
}

void setViewArea (int x, int y, int w, int h, float zmul) {
	setProjection (x*zmul, (x+w)*zmul, (-y+h)*zmul, -y*zmul);
}

void setSpriteViewArea (int x, int y, int w, int h, float zmul) {
	setProjection (x*zmul, (x+w)*zmul, (y-h)*zmul, y*zmul);
}

void setDrawingColour (float r, float g, float b, float a) {
	glUseProgram (program);
	glUniform4f (colourLocation, r, g, b, a);
}

// Draw count vertices of four floats each.
static void drawVertices (GLenum mode, const float * vertices, int count, bool textured) {
	glUseProgram (program);
	glUniform1i (texturedLocation, textured);
	glBindVertexArray (vertexArray);
	glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData (GL_ARRAY_BUFFER, count * 4 * sizeof (float), vertices, GL_STREAM_DRAW);
	glDrawArrays (mode, 0, count);
}

static void drawPoints (GLenum mode, const float * points, int count) {
	float * vertices = new float[count * 4];
	for (int i = 0; i < count; i ++) {
		vertices[i * 4] = points[i * 2];
		vertices[i * 4 + 1] = points[i * 2 + 1];
		vertices[i * 4 + 2] = vertices[i * 4 + 3] = 0;
	}
	drawVertices (mode, vertices, count, false);
	delete [] vertices;
}

void drawLines (const float * points, int count) {
	drawPoints (GL_LINES, points, count);
}

void drawLineLoop (const float * points, int count) {
	drawPoints (GL_LINE_LOOP, points, count);
}

void drawPolygon (const float * points, int count) {
	drawPoints (GL_TRIANGLE_FAN, points, count);
}

void drawTexture (GLuint texture, float x1, float y1, float x2, float y2,
		float tx1, float ty1, float tx2, float ty2) {
	const float vertices[] = {
		x1, y1, tx1, ty1,
		x2, y1, tx2, ty1,
		x2, y2, tx2, ty2,
		x1, y2, tx1, ty2
	};
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, texture);
	drawVertices (GL_TRIANGLE_FAN, vertices, 4, true);
}
