void main()
{
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;

	// Light
	gl_TexCoord[1] = gl_MultiTexCoord1;
	gl_FrontColor = gl_Color;
	gl_FrontSecondaryColor = gl_SecondaryColor;

}