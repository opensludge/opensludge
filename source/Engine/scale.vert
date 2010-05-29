/*
   
   Copyright (C) 2005 guest(r) - guest.r@gmail.com

   found at http://www.razyboard.com/system/morethread-smart-texture-mag-filter-for-ogl2-and-dosbox-pete_bernert-266904-5689051-0.html

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

uniform vec4 OGL2Param;
uniform vec4 OGL2Size;

void main()
{
// 1.0/2048.0 = 0.00048828125
// 1.0/1024.0 = 0.0009765625
//float x = OGL2Size.x * 0.0004883 * OGL2Param.x;
//float y = OGL2Size.y * 0.0009766 * OGL2Param.y;
float x = OGL2Size.x;
float y = OGL2Size.y;

vec2 sd1 = vec2( x,y) * 0.5; 
vec2 sd2 = vec2(-x,y) * 0.5;

vec2 ddx = vec2(  x, 0.0); 
vec2 ddy = vec2(0.0,   y);

gl_Position = ftransform();
gl_TexCoord[0] = gl_MultiTexCoord0;
gl_TexCoord[1].xy = gl_TexCoord[0].xy - sd1;
gl_TexCoord[2].xy = gl_TexCoord[0].xy - sd2;
gl_TexCoord[3].xy = gl_TexCoord[0].xy + sd1;
gl_TexCoord[4].xy = gl_TexCoord[0].xy + sd2;
gl_TexCoord[1].zw = gl_TexCoord[0].xy - ddy;
gl_TexCoord[2].zw = gl_TexCoord[0].xy + ddx;
gl_TexCoord[3].zw = gl_TexCoord[0].xy + ddy;
gl_TexCoord[4].zw = gl_TexCoord[0].xy - ddx;
}