attribute vec4 myVertex;
attribute vec2 myUV0;
attribute vec2 myUV1;
attribute vec2 myUV2;

uniform mat4 myPMVMatrix;

varying vec2 varCoord0;
varying vec2 varCoord1;
varying vec2 varCoord2;

void main() {
	varCoord0 = myUV0.st;
	varCoord1 = myUV1.st;
	varCoord2 = myUV2.st;
	gl_FrontColor = gl_Color;
	gl_FrontSecondaryColor = gl_SecondaryColor;
	gl_Position = myPMVMatrix * myVertex;
}
