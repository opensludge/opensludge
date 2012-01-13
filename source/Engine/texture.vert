attribute vec4 myVertex;
attribute vec2 myUV;

uniform mat4 myPMVMatrix;

varying vec2 varCoord;

void main(void)
{
	gl_Position = myPMVMatrix * myVertex;
	varCoord = myUV.st;
}

