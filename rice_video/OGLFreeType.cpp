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

//Include our header file.
#include "OGLError.h"
#include "OGLFreeType.h"

//OpenGL Headers 
#include "stdafx.h"
#include "glh_genext.h"
#include <GL/glu.h>
#include "OGLCombiner.h"

inline int next_p2 ( int a )
{
    int rval=1;
    while(rval<a) rval<<=1;
    return rval;
}

void makeDList ( FT_Face face, char ch, GLuint list_base, GLuint * tex_base ) 
{
    
    if(FT_Load_Glyph( face, FT_Get_Char_Index( face, ch ), FT_LOAD_DEFAULT )) fprintf(stderr, "FT_Load_Glyph failed");
    
    FT_Glyph glyph;
    if(FT_Get_Glyph( face->glyph, &glyph )) fprintf(stderr, "FT_Get_Glyph failed");
    
    FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
    
    FT_Bitmap& bitmap=bitmap_glyph->bitmap;
    
    int width = next_p2( bitmap.width );
    int height = next_p2( bitmap.rows );

	//Allocate memory for the texture data.
    GLubyte* expanded_data = new GLubyte[ 2 * width * height];
    
    for(int j=0; j <height;j++) 
    {
        for(int i=0; i < width; i++)
        {
            expanded_data[2*(i+j*width)]= expanded_data[2*(i+j*width)+1] = (i>=bitmap.width || j>=bitmap.rows) ? 0 : bitmap.buffer[i + bitmap.width*j];
        }
    }

    glBindTexture( GL_TEXTURE_2D, tex_base[ch]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height,0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data );
    
    delete [] expanded_data;
    
    glNewList(list_base+ch,GL_COMPILE);

    glBindTexture(GL_TEXTURE_2D,tex_base[ch]);
    
    glTranslatef(bitmap_glyph->left,0,0);
    
    glPushMatrix();
        glTranslatef(0,bitmap_glyph->top-bitmap.rows,0);
        
        float x=(float)bitmap.width / (float)width, y=(float)bitmap.rows / (float)height;
        glBegin(GL_QUADS);
            glTexCoord2d(0,0); glVertex2f(0,bitmap.rows);
            glTexCoord2d(0,y); glVertex2f(0,0);
            glTexCoord2d(x,y); glVertex2f(bitmap.width,0);
            glTexCoord2d(x,0); glVertex2f(bitmap.width,bitmap.rows);
        glEnd();
    glPopMatrix();
    glTranslatef(face->glyph->advance.x >> 6 ,0,0);
    
    glEndList();
}

Font::Font(const char * fname, unsigned int h) 
{
    fprintf(stderr, "OGLFreeType: Loading Font %s...\n",fname);
    textures = new GLuint[128];

    this->h=h;
    
    FT_Library library;
    if (FT_Init_FreeType( &library ))  fprintf(stderr,"FT_Init_FreeType failed\n");
    
    FT_Face face;
    
    if (FT_New_Face( library, fname, 0, &face )) fprintf(stderr,"FT_New_Face failed (there is probably a problem with your font file)\n");
    
    FT_Set_Char_Size( face, h << 6, h << 6, 96, 96);
    
    list_base=glGenLists(128);
    OGLCheckErrors();
    glGenTextures( 128, textures );
    OGLCheckErrors();
    
    for(unsigned char i=0;i<128;i++) makeDList(face,i,list_base,textures);
    for(unsigned char i=0;i<128;i++)
    {
        if(FT_Load_Glyph( face, FT_Get_Char_Index( face, i ), FT_LOAD_DEFAULT )) fprintf(stderr, "FT_Load_Glyph failed\n");
        w[i] = face->glyph->advance.x;
    }
    
    FT_Done_Face(face);
    
    FT_Done_FreeType(library);
}

Font::~Font() 
{
    glDeleteLists(list_base,128);
    glDeleteTextures(128,textures);
    delete [] textures;
}

inline void pushProjectionMatrix() 
{
    glPushAttrib(GL_TRANSFORM_BIT);
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(viewport[0],viewport[2],viewport[1],viewport[3]);
    glPopAttrib();
}

inline void PopProjectionMatrix() 
{
    glPushAttrib(GL_TRANSFORM_BIT);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    glPopAttrib();
}

OGLFT_Boundary PreflightTextSize(const Font &ft_font, const char *fmt, ...)
{
    OGLFT_Boundary size;
    char text[256];
    char *pointer = &text[0];
    va_list ap;

    if (fmt == NULL) *text=0;
    else 
    {
        va_start(ap, fmt);
        vsprintf(text, fmt, ap);
        va_end(ap);
    }

    size.H = ft_font.h;
    while(*pointer)
    {
        size.W += ft_font.w[*pointer];
        pointer++;
    }
    
    return size;
}

void glPrint(const Font &ft_font, float x, float y, float color[4], const char *fmt, ...)  
{
    pushProjectionMatrix();					
	
    GLuint font=ft_font.list_base;
    float h = ft_font.h/.63f;
    
    char text[256];
    va_list	ap;

    if (fmt == NULL) *text=0;
    else 
    {
        va_start(ap, fmt);
        vsprintf(text, fmt, ap);
        va_end(ap);
    }
    
    const char *start_line=text;
    vector<string> lines;

    const char * c = text;;

    for(;*c;c++) 
    {
        if(*c=='\n') 
        {
            string line;
            for(const char *n=start_line;n<c;n++) line.append(1,*n);
            lines.push_back(line);
            start_line=c+1;
        }
    }
    
    if(start_line) 
    {
        string line;
        for(const char *n=start_line;n<c;n++) line.append(1,*n);
        lines.push_back(line);
    }

    CRender::g_pRender->m_pColorCombiner->DisableCombiner();
    glPushClientAttrib(GL_ALL_CLIENT_ATTRIB_BITS);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_EDGE_FLAG_ARRAY);
    glDisableClientState(GL_INDEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_FOG_COORDINATE_ARRAY_EXT);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_TEXTURE_2D);
    glBlendColorEXT(0,0,0,0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_2D);
    glListBase(font);
    
    glColor4fv(color);

    float modelview_matrix[16];	
    glMatrixMode(GL_MODELVIEW);
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix);
    
    for(unsigned int i=0;i<lines.size();i++) 
    {
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(x,y-h*i,0);
        glMultMatrixf(modelview_matrix);
        
        glCallLists(lines[i].length(), GL_UNSIGNED_BYTE, lines[i].c_str());
        
        glPopMatrix();
    }

    glPopAttrib();
    glPopClientAttrib();

    PopProjectionMatrix();
    OGLCheckErrors();
}
