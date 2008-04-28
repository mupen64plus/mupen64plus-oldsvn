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

#ifndef __OGLFREETYPE_H__
#define __OGLFREETYPE_H__

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include <vector>
#include <string>

using std::vector;
using std::string;

class Font 
{
    public:
        Font(const char * fname, unsigned int h);
        ~Font();

        float h;
        float w[256];
        unsigned int *textures;
        unsigned int list_base;
};

struct OGLFT_Boundary
{
    float W,H;
};
void glPrint(const Font &ft_font, float x, float y, float color[4], const char *fmt, ...);
OGLFT_Boundary PreflightTextSize(const Font &ft_font, const char *fmt, ...);


#endif
