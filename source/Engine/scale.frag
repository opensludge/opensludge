/*
   Inspired by a shader by guest(r) - guest.r@gmail.com - that was found at
   http://www.razyboard.com/system/morethread-smart-texture-mag-filter-for-ogl2-and-dosbox-pete_bernert-266904-5689051-0.html
*/

uniform sampler2D Texture;
uniform sampler2D lightTexture;
uniform bool useLightTexture;
uniform float scale;

void main()
{
	vec2 fw = fwidth(gl_TexCoord[0].xy)*scale;

	vec2 sd1 = vec2( fw.x,fw.y);
	vec2 sd2 = vec2(-fw.x,fw.y);

	vec4 c11 = texture2D(Texture, gl_TexCoord[0].xy);
		
	vec4 s00 = texture2D(Texture, gl_TexCoord[0].xy-sd1);
	vec4 s20 = texture2D(Texture, gl_TexCoord[0].xy-sd2);
	vec4 s22 = texture2D(Texture, gl_TexCoord[0].xy+sd1);
	vec4 s02 = texture2D(Texture, gl_TexCoord[0].xy+sd2);   

	vec4 dt = vec4(1.0,1.0,1.0,1.0);
		
	float m1=dot(abs(s00-s22),dt)+0.0001;
	float m2=dot(abs(s02-s20),dt)+0.0001;

	vec4 temp1 = m2*(s00 + s22) + m1*(s02 + s20);

//	gl_FragColor = (temp1/(m1+m2)) * 0.5;
	gl_FragColor = c11*0.333333 + (temp1/(m1+m2)) * 0.333333;

	if (gl_FragColor.a<0.001) discard;
	
	vec3 col;
	if (useLightTexture) {
		vec4 texture1 = texture2D (lightTexture, gl_TexCoord[1].xy);
		col = texture1.rgb * gl_FragColor.rgb;
	} else {
		col = gl_Color.rgb * gl_FragColor.rgb;
	}
	col += vec3(gl_SecondaryColor);
	gl_FragColor = vec4 (col, gl_Color.a * gl_FragColor.a);
}

