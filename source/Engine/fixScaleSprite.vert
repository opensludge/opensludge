void main() {
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
	gl_TexCoord[2] = gl_MultiTexCoord2;
	gl_FrontColor = gl_Color;
	gl_FrontSecondaryColor = gl_SecondaryColor;
	gl_Position = ftransform();
}