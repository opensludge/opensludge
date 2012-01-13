attribute vec4 myUV;

varying vec2 varCoord;

void main(void)
{
	gl_Position = ftransform();
	varCoord = myUV.st;
}

