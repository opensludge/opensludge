attribute vec4 myVertex;
attribute vec2 myUV;

uniform mat4 myPMVMatrix;
uniform mat4 myProjectionMatrix;
uniform mat4 myModelViewMatrix;

varying vec2 varCoord;

void main(void)
{
	gl_Position = myProjectionMatrix * myModelViewMatrix * myVertex;
	varCoord = myUV.st;
}

