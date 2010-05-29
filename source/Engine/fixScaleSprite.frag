uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform bool useLightTexture;

void main()
{
	vec4 texture = texture2D (tex0, gl_TexCoord[0].xy);
	vec4 texture2 = texture2D (tex2, gl_TexCoord[2].xy);
	vec3 col;
	if (useLightTexture) {
		vec4 texture1 = texture2D (tex1, gl_TexCoord[1].xy);
		col = texture1.rgb * texture.rgb;
	} else {
		col = gl_Color.rgb * texture.rgb;
	}
	col += vec3(gl_SecondaryColor);
	vec4 color = vec4 (col, gl_Color.a * texture.a);
	col = mix (texture2.rgb, color.rgb, color.a);
	gl_FragColor = vec4 (col, max(texture.a, texture2.a));
}

