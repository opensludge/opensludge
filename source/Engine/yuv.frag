uniform sampler2D Ytex;
uniform sampler2D Utex;
uniform sampler2D Vtex;

void main()
{
	float y, u, v, r, g, b;

	y=texture2D(Ytex, gl_TexCoord[0].xy).a;
	u=texture2D(Utex, gl_TexCoord[0].xy).a;
	v=texture2D(Vtex, gl_TexCoord[0].xy).a;
	
	y=1.1643*(y-0.0625);
	u=u-0.5;
	v=v-0.5;

	r=y+1.5958*v;
	g=y-0.39173*u-0.81290*v;
	b=y+2.017*u;

	gl_FragColor=vec4(r,g,b,1.0);
}

