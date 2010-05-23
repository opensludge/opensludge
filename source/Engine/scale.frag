/*
   The GLSmartTexMag filter shader
   
   found at http://www.razyboard.com/system/morethread-smart-texture-mag-filter-for-ogl2-and-dosbox-pete_bernert-266904-5689051-0.html
   
   Copyright (C) 2009 guest(r) - guest.r@gmail.com

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/


uniform sampler2D OGL2Texture;

void main()
{
    vec3 c11 = texture(OGL2Texture, gl_TexCoord[0].xy).xyz;
    vec3 s00 = texture(OGL2Texture, gl_TexCoord[1].xy).xyz; 
    vec3 s20 = texture(OGL2Texture, gl_TexCoord[2].xy).xyz; 
    vec3 s22 = texture(OGL2Texture, gl_TexCoord[3].xy).xyz; 
    vec3 s02 = texture(OGL2Texture, gl_TexCoord[4].xy).xyz;     
    vec3 c01 = texture(OGL2Texture, gl_TexCoord[1].zw).xyz; 
    vec3 c21 = texture(OGL2Texture, gl_TexCoord[2].zw).xyz; 
    vec3 c10 = texture(OGL2Texture, gl_TexCoord[3].zw).xyz; 
    vec3 c12 = texture(OGL2Texture, gl_TexCoord[4].zw).xyz;  
    vec3 dt = vec3(1.0,1.0,1.0);

    float hl=dot(abs(c01-c21),dt)+0.0001;
    float vl=dot(abs(c10-c12),dt)+0.0001;
    float m1=dot(abs(s00-s22),dt)+0.0001;
    float m2=dot(abs(s02-s20),dt)+0.0001;            
    
    vec3 temp1 = m2*(s00 + s22) + m1*(s02 + s20);
    vec3 temp2 = hl*(c10 + c12) + vl*(c01 + c21);
    
    c11 = (temp2/(hl+vl) + c11 + c11) * 0.083333 + (temp1/(m1+m2)) * 0.333333;

    vec3 mn1 = min(min(s00,c01),s02);
    vec3 mn2 = min(min(c10,c11),c12);
    vec3 mn3 = min(min(s20,c21),s22);

    vec3 mx1 = max(max(s00,c01),s02);
    vec3 mx2 = max(max(c10,c11),c12);
    vec3 mx3 = max(max(s20,c21),s22);

    mn1 = min(min(mn1,mn2),mn3);
    mx1 = max(max(mx1,mx2),mx3);

    // float filterparam = 3.0;     

    vec3 dif1 = 0.0001*dt + abs(c11-mn1);
    vec3 dif2 = 0.0001*dt + abs(c11-mx1);

    // dif = pow(dif, filterparam)
    dif1 = dif1 * dif1 * dif1;
    dif2 = dif2 * dif2 * dif2;

    c11 = (dif1 * mx1 + dif2 * mn1) / (dif1 + dif2);
        
    // 16-255    
    // gl_FragColor.xyz = c11 * 1.0669456 - 0.0669456;    
    // 16-235
    gl_FragColor.xyz = c11 * 1.16438356 - 0.07305936;    
    
}