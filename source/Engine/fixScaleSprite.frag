uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform bool useLightTexture;

varying vec2 varCoord0;
varying vec2 varCoord1;
varying vec2 varCoord2;

void main()
{
	vec4 texture = texture2D (tex0, varCoord0);
	vec4 texture2 = texture2D (tex2, varCoord2);
	vec3 col;
	if (useLightTexture) {
		vec4 texture1 = texture2D (tex1, varCoord1);
		col = texture1.rgb * texture.rgb;
	} else {
		col = gl_Color.rgb * texture.rgb;
	}
	col += vec3(gl_SecondaryColor);
	vec4 color = vec4 (col, gl_Color.a * texture.a);
	col = mix (texture2.rgb, color.rgb, color.a);
	gl_FragColor = vec4 (col, max(texture.a, texture2.a));
}

