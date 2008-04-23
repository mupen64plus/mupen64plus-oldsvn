/*
Copyright (C) 2003 Sven Olsen
Copyright (C) 2008 nmn

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


//FreeType Headers
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

//OpenGL Headers 
#include <GL/gl.h>
#include <GL/glu.h>

//Some STL headers
#include <vector>
#include <string>

//Using the STL exception library increases the
//chances that someone else using our code will corretly
//catch any exceptions that we throw.
#include <stdexcept>

//Inside of this namespace, give ourselves the ability
//to write just "vector" instead of "std::vector"
using std::vector;

//Ditto for string.
using std::string;

//This holds all of the information related to any
//freetype font that we want to create.  
struct Font {
	float h;			///< Holds the height of the font.
	GLuint * textures;	///< Holds the texture id's 
	GLuint list_base;	///< Holds the first display list id

	//The init function will create a font of
	//of the height h from the file fname.
	Font(const char * fname, unsigned int h);

	//Free all the resources assosiated with the font.
	~Font();
};

//The flagship function of the library - this thing will print
//out text at window coordinates x,y, using the font ft_font.
//The current modelview matrix will also be applied to the text. 
void glPrint(const Font &ft_font, float x, float y, const char *fmt, ...);


