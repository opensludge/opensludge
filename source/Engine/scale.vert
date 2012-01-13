attribute vec4 myVertex;
attribute vec2 myUV0;
attribute vec2 myUV1;

uniform mat4 myProjectionMatrix;
uniform mat4 myModelViewMatrix;

varying vec2 varCoord0;
varying vec2 varCoord1;

void main()
{
	gl_Position = myProjectionMatrix * myModelViewMatrix * myVertex;
	varCoord0 = myUV0.st;

	// Light
	varCoord1 = myUV1.st;
	gl_FrontColor = gl_Color;
	gl_FrontSecondaryColor = gl_SecondaryColor;

}
