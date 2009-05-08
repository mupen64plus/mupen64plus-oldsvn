/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Copyright (C) 2009 icepir8                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <windows.h>
#include <GL/gl.h>
//#include <glext.h>
#include <GL/glu.h>

#include "rdp_registers.h"
#include "rdp_gl.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "3dmath.h"
#include "Texture.h"
//** Globals
extern char output[1024];

#define kListEndLights      1
#define kListBeginLights    9
//#define OPENGL_LIGHTS

//** for Lighting
BOOL refresh_lights = TRUE;
BOOL refresh_matrix = TRUE;
MATRIX4 view_matrix, invers;
VECTOR Light_Vector[8];




//new june 16, need this for some combine modes.
/******************************************************************************\
*                                                                              *
*   Lighting                                                                   *
*                                                                              *
\******************************************************************************/

void calculate_light_vectors()
{
    unsigned int lidx;

#ifdef OPENGL_LIGHTS

    GLint lightnum;
    GLfloat tLight[4];
    GLfloat tcolor[4]={1.0,1.0,1.0,1.0};
    GLfloat tcolorz[4]={0.0,0.0,0.0,0.0};

    //glMatrixMode(GL_PROJECTION);
    //glPushMatrix();
    //glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
/*  tcolor[0] /= (rdp_reg.lights + 1);
    tcolor[1] /= (rdp_reg.lights + 1);
    tcolor[2] /= (rdp_reg.lights + 1);
*/
    glMaterialfv(GL_FRONT,GL_DIFFUSE,tcolor);
    glMaterialfv(GL_FRONT,GL_AMBIENT,tcolor);

    for (lidx = 0; lidx < 8; lidx++)
    {
                glDisable(GL_LIGHT0 + lidx);
    }

//  glCallList(kListBeginLights);

    for (lidx = 0; lidx < rdp_reg.lights; lidx++)
    {
        lightnum = GL_LIGHT0 + lidx;

        glEnable(lightnum);

        tLight[0] = rdp_reg.light[lidx].x;
        tLight[1] = rdp_reg.light[lidx].y;
        tLight[2] = rdp_reg.light[lidx].z;
        tLight[3] = 0;

        tcolor[0] = rdp_reg.light[lidx].r;
        tcolor[1] = rdp_reg.light[lidx].g;
        tcolor[2] = rdp_reg.light[lidx].b;
        tcolor[3] = rdp_reg.light[lidx].a;

        glLightfv(lightnum,GL_SPECULAR,tcolorz);
        glLightfv(lightnum,GL_DIFFUSE,tcolor);
        glLightfv(lightnum,GL_POSITION,tLight);

    }

    lidx = rdp_reg.lights;

    tcolor[0] = rdp_reg.light[lidx].r;
    tcolor[1] = rdp_reg.light[lidx].g;
    tcolor[2] = rdp_reg.light[lidx].b;
    tcolor[3] = rdp_reg.light[lidx].a;

    glEnable(GL_LIGHT0 + lidx);

    glLightfv(GL_LIGHT0 + lidx,GL_AMBIENT,tcolor);
    glLightfv(GL_LIGHT0 + lidx,GL_SPECULAR,tcolorz);
    glLightfv(GL_LIGHT0 + lidx,GL_DIFFUSE,tcolorz);

    //glPopAttrib();
    glPopMatrix();
    //glMatrixMode(GL_PROJECTION);
    //glPopMatrix();
#else

    for (lidx = 0; lidx < rdp_reg.lights; lidx++)
    {   
        TranformVector( &rdp_reg.light[lidx].x, Light_Vector[lidx], invers );
        NormalizeVector( Light_Vector[lidx] );
    }

#endif
    refresh_lights = FALSE;
}

#define NORMAL_LIGHTING

void math_lighting(t_vtx_tn *vtx, float color[4])
{
//  1. Inverse modelviematrix
//  2. Multiply lightvector invers-matrix
//  3. Normalize new vector
//  4. lightfactor = DotProduct ( vertex, new vector)
//  5. if new lightfactor < 0 ; new lightfactor = 0
//  6. light = light_color * lightfactor

    unsigned int lidx;
    VECTOR tmp_vec1;

    tmp_vec1[0] = (float)vtx->nx;
    tmp_vec1[1] = (float)vtx->ny;
    tmp_vec1[2] = (float)vtx->nz;
    NormalizeVector(tmp_vec1);

// The spot-lights ...
    for (lidx = 0;lidx < rdp_reg.lights; lidx++)
    {
        float light_intensity;

#ifdef NORMAL_LIGHTING
        light_intensity = DotProduct( Light_Vector[lidx], tmp_vec1 );       
#else
        VECTOR tmp1, tmp2;

        tmp_vec1[0] = (float)vtx->nx;
        tmp_vec1[1] = (float)vtx->ny;
        tmp_vec1[2] = (float)vtx->nz;

        TranformVector(tmp_vec1, tmp1, view_matrix);
        //NormalizeVector(tmp1);
        light_intensity = DotProduct( Light_Vector[lidx], tmp1 );
#endif      
        
        if (light_intensity < 0) light_intensity = 0;
                        
        color[0] += rdp_reg.light[lidx].r * light_intensity;
        color[1] += rdp_reg.light[lidx].g * light_intensity;
        color[2] += rdp_reg.light[lidx].b * light_intensity;                
//      color[3] += vtx->a / 255.0f;
        color[3] = 1.0f;
    }
// We need ambient-light here ...               
    color[0] += rdp_reg.light[rdp_reg.lights].r;
    color[1] += rdp_reg.light[rdp_reg.lights].g;
    color[2] += rdp_reg.light[rdp_reg.lights].b;
//  color[3] += vtx->a / 255.0f;
    color[3] = 1.0f;

    return;
}

void UpdateLights()
{
    int lidx;
    float tmp_light[4];
    float tmp_light2[4];

    for (lidx = 0;lidx < (int)rdp_reg.lights; lidx++)
    {
        tmp_light[0] = rdp_reg.light[lidx].x;
        tmp_light[1] = rdp_reg.light[lidx].y;
        tmp_light[2] = rdp_reg.light[lidx].z;

        InversTranformVector((float*)tmp_light, (float*)&tmp_light2);

        rdp_reg.light[lidx].mx = tmp_light2[0];
        rdp_reg.light[lidx].my = tmp_light2[1];
        rdp_reg.light[lidx].mz = tmp_light2[2];
    }
    refresh_lights = FALSE;
}

void math_lightingN(t_vtx *vtx, float color[4])
{
//  1. Inverse modelviematrix
//  2. Multiply lightvector invers-matrix
//  3. Normalize new vector
//  4. lightfactor = DotProduct ( vertex, new vector)
//  5. if new lightfactor < 0 ; new lightfactor = 0
//  6. light = light_color * lightfactor

    unsigned int lidx;
    VECTOR tmp_vec1;
    VECTOR tmp_light;
    float light_intensity;

    tmp_vec1[0] = (float)vtx->n1;
    tmp_vec1[1] = (float)vtx->n2;
    tmp_vec1[2] = (float)vtx->n3;

    // We need ambient-light here ...               
    color[0] = rdp_reg.light[rdp_reg.ambient_light].r;
    color[1] = rdp_reg.light[rdp_reg.ambient_light].g;
    color[2] = rdp_reg.light[rdp_reg.ambient_light].b;
    color[3] = 1.0f;
    //NormalizeVector(tmp_vec1);
    if (refresh_lights) UpdateLights();

// The spot-lights ...
    for (lidx = 0;lidx < rdp_reg.lights; lidx++)
    {
        tmp_light[0] = rdp_reg.light[lidx].mx;
        tmp_light[1] = rdp_reg.light[lidx].my;
        tmp_light[2] = rdp_reg.light[lidx].mz;
        //NormalizeVector(tmp_light);//rdp_reg.light[lidx]

        //light_intensity = DotProduct( Light_Vector[lidx], tmp_vec1 );     
        light_intensity = DotProduct(tmp_light, tmp_vec1);      

        if (light_intensity < 0) light_intensity = 0;
//      if (light_intensity > 1) light_intensity = 1;
                        
        color[0] += rdp_reg.light[lidx].r * light_intensity;
        color[1] += rdp_reg.light[lidx].g * light_intensity;
        color[2] += rdp_reg.light[lidx].b * light_intensity;                
//      color[3] = 1.0f;
    }


    return;
}

void math_lightingMM(t_vtx *vtx, float color[4])
{
//  1. Inverse modelviematrix
//  2. Multiply lightvector invers-matrix
//  3. Normalize new vector
//  4. lightfactor = DotProduct ( vertex, new vector)
//  5. if new lightfactor < 0 ; new lightfactor = 0
//  6. light = light_color * lightfactor

    unsigned int lidx;
    VECTOR tmp_vec1;
    VECTOR tmp_light;
    float light_intensity;

    tmp_vec1[0] = (float)vtx->n1;
    tmp_vec1[1] = (float)vtx->n2;
    tmp_vec1[2] = (float)vtx->n3;

    // We need ambient-light here ...               
    color[0] = rdp_reg.light[rdp_reg.ambient_light].r;
    color[1] = rdp_reg.light[rdp_reg.ambient_light].g;
    color[2] = rdp_reg.light[rdp_reg.ambient_light].b;
    color[3] = 1.0f;
    //NormalizeVector(tmp_vec1);
    if (refresh_lights) UpdateLights();

// The spot-lights ...
    for (lidx = 0;lidx < rdp_reg.lights; lidx++)
    {
        if (rdp_reg.light[lidx].w == 0)
        {
            tmp_light[0] = rdp_reg.light[lidx].mx;
            tmp_light[1] = rdp_reg.light[lidx].my;
            tmp_light[2] = rdp_reg.light[lidx].mz;

            light_intensity = DotProduct(tmp_light, tmp_vec1);      

            if (light_intensity < 0) light_intensity = 0;
            if (light_intensity > 1) light_intensity = 1;
                        
            color[0] += rdp_reg.light[lidx].r * light_intensity;
            color[1] += rdp_reg.light[lidx].g * light_intensity;
            color[2] += rdp_reg.light[lidx].b * light_intensity;                
        }
        else
        {
            VECTOR dir;
            float d2;
            dir[0] = rdp_reg.light[lidx].x - (float)vtx->x;
            dir[1] = rdp_reg.light[lidx].y - (float)vtx->y;
            dir[2] = rdp_reg.light[lidx].z - (float)vtx->z;

            d2 = (float)sqrt(dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2]);
            
            dir[0] /= d2;
            dir[1] /= d2;
            dir[2] /= d2;

            light_intensity = DotProduct(dir, tmp_vec1);        

            if (light_intensity < 0) light_intensity = 0;

            if (light_intensity > 0)
            {
                float f = d2/750000;//113510;//15000*50;
                f = 1 - min(f,1);
                light_intensity *= f*f;

                color[0] += rdp_reg.light[lidx].r * light_intensity;
                color[1] += rdp_reg.light[lidx].g * light_intensity;
                color[2] += rdp_reg.light[lidx].b * light_intensity;
            }
        }
    }


    return;
}

