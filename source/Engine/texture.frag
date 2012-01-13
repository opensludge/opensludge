uniform sampler2D sampler2d;
uniform bool zBuffer;
uniform float zBufferLayer;

varying vec2 varCoord;

void main(void)
{
	vec4 col = texture2D(sampler2d, varCoord);
	if (zBuffer && col.a < 0.0625*zBufferLayer-0.03)
	{
		discard;
	}
	gl_FragColor = col;
}

