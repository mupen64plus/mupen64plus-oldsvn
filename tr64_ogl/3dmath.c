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
#include <math.h>
#include <stdio.h>
#include "rdp_registers.h"
#include "3dmath.h"

typedef float MATRIX3[9];
typedef float MATRIX4[16];
typedef float VFLOAT;
typedef float VECTOR[4];

static void PrintMatrix4(MATRIX4 Matrix);
static void m4_submat( MATRIX4 mr, MATRIX3 mb, int i, int j );
static VFLOAT m4_det( MATRIX4 mr );
static int m4_inverse( MATRIX4 mr, MATRIX4 ma );
static VFLOAT m3_det( MATRIX3 mat );

//extern HWND hGraphics;

extern BOOL refresh_matrix;

static void PrintMatrix4(MATRIX4 Matrix)
{
    char text[1024];
    sprintf(text,"%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n",
                    Matrix[0],  Matrix[1],  Matrix[2],  Matrix[3],
                    Matrix[4],  Matrix[5],  Matrix[6],  Matrix[7],
                    Matrix[8],  Matrix[9],  Matrix[10], Matrix[11],
                    Matrix[12], Matrix[13], Matrix[14], Matrix[15]);
//  MessageBox(hGraphics, text, "Matrix", MB_OK);
}

static void PrintVector(VECTOR vec)
{
    char text[1024];
    sprintf(text,"%f %f %f \n",
                    vec[0], vec[1], vec[2]);
//  MessageBox(hGraphics, text, "Vector", MB_OK);
}

static void PrintFloat(float f)
{
    char text[1024];
    sprintf(text,"%f\n",f);
//  MessageBox(hGraphics, text, "Matrix", MB_OK);
}


static void m4_submat( MATRIX4 mr, MATRIX3 mb, int i, int j )
{
    int ti, tj, idst, jdst;
    for ( ti = 0; ti < 4; ti++ )
    {
    if ( ti < i )
        idst = ti;
    else if ( ti > i )
        idst = ti-1;

    for ( tj = 0; tj < 4; tj++ )
    {
        if ( tj < j )
            jdst = tj;
        else if ( tj > j )
            jdst = tj-1;

        if ( ti != i && tj != j )
            mb[idst*3 + jdst] = mr[ti*4 + tj ];
        }
    }
}

/*
    --------------------------

  The determinant of a 4x4 matrix can be calculated as follows:

    --------------------------
*/
static VFLOAT m4_det( MATRIX4 mr )
{
    VFLOAT  det, result = 0, i = 1;
    MATRIX3 msub3;
    int     n;

    for ( n = 0; n < 4; n++, i *= -1 )
    {
        m4_submat( mr, msub3, 0, n );

        det     = m3_det( msub3 );
        result += mr[n] * det * i;
    }

    return( result );
}

/*    --------------------------

  And the inverse can be calculated as follows:

    --------------------------
*/
static int m4_inverse( MATRIX4 mr, MATRIX4 ma )
{
    int     i, j, sign;
    VFLOAT  mdet = m4_det( ma );
    MATRIX3 mtemp;


    if ( fabs( mdet ) < 0.0005 )
        return( 0 );

    for ( i = 0; i < 4; i++ )
        for ( j = 0; j < 4; j++ )
        {
            sign = 1 - ( (i +j) % 2 ) * 2;
            m4_submat( ma, mtemp, i, j );
            mr[i*4+j] = ( m3_det( mtemp ) * sign ) / mdet;
        }

    return( 1 );
}

int m3_inverse( MATRIX4 mr, MATRIX4 ma )
{
//  int     i, j, sign;
//  VFLOAT  mdet = m4_det( ma );
//  MATRIX3 mtemp;

/*
    mr[0 * 4 + 0] = +(ma[1 * 4 + 1]*ma[2 * 4 + 2]-ma[1 * 4 + 2]*ma[2 * 4 + 1]);
    mr[0 * 4 + 1] = -(ma[0 * 4 + 1]*ma[2 * 4 + 2]-ma[0 * 4 + 2]*ma[2 * 4 + 1]);
    mr[0 * 4 + 2] = +(ma[0 * 4 + 1]*ma[1 * 4 + 2]-ma[0 * 4 + 2]*ma[1 * 4 + 1]);
    mr[1 * 4 + 0] = -(ma[1 * 4 + 0]*ma[2 * 4 + 2]-ma[1 * 4 + 2]*ma[2 * 4 + 0]);
    mr[1 * 4 + 1] = +(ma[0 * 4 + 0]*ma[2 * 4 + 2]-ma[0 * 4 + 2]*ma[2 * 4 + 0]);
    mr[1 * 4 + 2] = -(ma[0 * 4 + 0]*ma[1 * 4 + 2]-ma[0 * 4 + 2]*ma[1 * 4 + 0]);
    mr[2 * 4 + 0] = +(ma[1 * 4 + 0]*ma[2 * 4 + 1]-ma[1 * 4 + 1]*ma[2 * 4 + 0]);
    mr[2 * 4 + 1] = -(ma[0 * 4 + 0]*ma[2 * 4 + 1]-ma[0 * 4 + 1]*ma[2 * 4 + 0]);
    mr[2 * 4 + 2] = +(ma[0 * 4 + 0]*ma[1 * 4 + 1]-ma[0 * 4 + 1]*ma[1 * 4 + 0]);

    mr[0]  = +(ma[5] * ma[10] - ma[6] * ma[9]);
    mr[1]  = -(ma[1] * ma[10] - ma[2] * ma[9]);
    mr[2]  = +(ma[1] * ma[6]  - ma[2] * ma[5]);

    mr[4]  = -(ma[4] * ma[10] - ma[6] * ma[8]);
    mr[5]  = +(ma[0] * ma[10] - ma[2] * ma[8]);
    mr[6]  = -(ma[0] * ma[6] -  ma[2] * ma[4]);
    
    mr[8]  = +(ma[4] * ma[9] -  ma[5] * ma[8]);
    mr[9]  = -(ma[0] * ma[9] -  ma[1] * ma[8]);
    mr[10] = +(ma[0] * ma[5] -  ma[1] * ma[4]);

    mr[0]  =  ma[0];
    mr[1]  = -ma[1];
    mr[2]  = -ma[2];

    mr[4]  = -ma[4];
    mr[5]  =  ma[5];
    mr[6]  = -ma[6];
    
    mr[8]  = -ma[8];
    mr[9]  = -ma[9];
    mr[10] =  ma[10];
*/

    mr[0]  =  ma[0];
    mr[5]  =  ma[5];
    mr[10] =  ma[10];

    mr[1]  =  ma[4];
    mr[4]  =  ma[1];

    mr[2]  =  ma[8];
    mr[8]  =  ma[2];

    mr[6]  =  ma[9];
    mr[9]  =  ma[6];

    return( 1 );
}

static VFLOAT m3_det( MATRIX3 mat )
{
    VFLOAT det;

    det = mat[0] * ( mat[4]*mat[8] - mat[7]*mat[5] )
        - mat[1] * ( mat[3]*mat[8] - mat[6]*mat[5] )
        + mat[2] * ( mat[3]*mat[7] - mat[6]*mat[4] );

    return( det );
}

void TranformVector(float *src, float *dst, MATRIX4 mat)
{
    dst[0] = mat[ 0]*src[0]
           + mat[ 4]*src[1]
           + mat[ 8]*src[2];

    dst[1] = mat[ 1]*src[0]
           + mat[ 5]*src[1]
           + mat[ 9]*src[2];

    dst[2] = mat[ 2]*src[0]
           + mat[ 6]*src[1]
           + mat[10]*src[2];
}

void InversTranformVector(float *src, float *dst)
{
    float x = src[0];
    float y = src[1];
    float z = src[2];

//  dst[0] = src[0];
//  dst[1] = src[1];
//  dst[2] = src[2];

    dst[0]  = x * m_stack[m_idx][0][0] + 
              y * m_stack[m_idx][0][1] + 
              z * m_stack[m_idx][0][2];

    dst[1]  = x * m_stack[m_idx][1][0] + 
              y * m_stack[m_idx][1][1] + 
              z * m_stack[m_idx][1][2];

    dst[2]  = x * m_stack[m_idx][2][0] + 
              y * m_stack[m_idx][2][1] + 
              z * m_stack[m_idx][2][2];
/**/
    NormalizeVector(dst);
}

float DotProduct(float *v1, float *v2)
{
    float result;

    result = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];

    return(result);
}

void CrossProduct(float *v1, float *v2, float *v3)
{
    v3[0] = v1[1]*v2[2] - v1[2]*v2[1];
    v3[1] = v1[2]*v2[0] - v1[0]*v2[2];
    v3[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

void NormalizeVector(float *v)
{
    float len;
    len = (float)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (len !=0)
    {
        v[0]/=len;
        v[1]/=len;
        v[2]/=len;
    }
    else
    {
        v[0] = 0;
        v[1] = 0;
        v[2] = 0;
    }
}

BOOL compare_matrix4(MATRIX4 m1, MATRIX4 m2)
{
    int i,j;
    for (i=0; i<4; i++)
        for (j=0; j<4; j++)
            if (m1[i*4+j] != m2[i*4+j]) return(FALSE);

    return(TRUE);
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// own matrix stuff... faster that way
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

//#define MAXSTACK 512 
#define MAXSTACK 16 // nintendo 64 uses 10 extra is just for safety.

float          m_stack[MAXSTACK][4][4];
float          prj_mtrx[4][4] =    {{1.0f,0.0f,0.0f,0.0f},
                                    {0.0f,1.0f,0.0f,0.0f},
                                    {0.0f,0.0f,1.0f,0.0f},
                                    {0.0f,0.0f,0.0f,1.0f}};
float          cmb_mtrx[4][4] =    {{1.0f,0.0f,0.0f,0.0f},
                                    {0.0f,1.0f,0.0f,0.0f},
                                    {0.0f,0.0f,1.0f,0.0f},
                                    {0.0f,0.0f,0.0f,1.0f}};
unsigned short m_idx=0;

/////////////////////////////////////////////////////////

float z_add=0.0f;

void transform_vector(float * vd,float x0,float y0,float z0)
{
   x0 = vd[0];
   y0 = vd[1];
   z0 = vd[2];

   
 *vd     = x0 * m_stack[m_idx][0][0] + 
           y0 * m_stack[m_idx][1][0] + 
           z0 * m_stack[m_idx][2][0] +
                m_stack[m_idx][3][0];

 *(vd+1) = x0 * m_stack[m_idx][0][1] + 
           y0 * m_stack[m_idx][1][1] + 
           z0 * m_stack[m_idx][2][1] +
                m_stack[m_idx][3][1];

 *(vd+2) = x0 * m_stack[m_idx][0][2] + 
           y0 * m_stack[m_idx][1][2] + 
           z0 * m_stack[m_idx][2][2] +
                m_stack[m_idx][3][2];
}

void transform_vector2(float * vd,float x0,float y0,float z0, float w0)
{

    x0 = vd[0];
    y0 = vd[1];
    z0 = vd[2];
    w0 = vd[3];
   
    vd[0]   = x0 * m_stack[m_idx][0][0] +
              y0 * m_stack[m_idx][1][0] + 
              z0 * m_stack[m_idx][2][0];

    vd[1]   = x0 * m_stack[m_idx][0][1] + 
              y0 * m_stack[m_idx][1][1] + 
              z0 * m_stack[m_idx][2][1];

    vd[2]   = x0 * m_stack[m_idx][0][2] + 
              y0 * m_stack[m_idx][1][2] + 
              z0 * m_stack[m_idx][2][2];

    vd[3]   = x0 * m_stack[m_idx][0][3] + 
              y0 * m_stack[m_idx][1][3] + 
              z0 * m_stack[m_idx][2][3];

    vd[0] += m_stack[m_idx][3][0];
    vd[1] += m_stack[m_idx][3][1];
    vd[2] += m_stack[m_idx][3][2];
    vd[3] += m_stack[m_idx][3][3];
}

void project_vector(float * vd,float x0,float y0,float z0)
{
/*
 if(z_add)                                             // zbuffer hack... mmm
  {
   x0+=z_add;
   y0+=z_add;
   y0+=z_add;
  }
*/
 *vd     = x0 * prj_mtrx[0][0] + 
           y0 * prj_mtrx[1][0] + 
           z0 * prj_mtrx[2][0] +
                prj_mtrx[3][0];

 *(vd+1) = x0 * prj_mtrx[0][1] + 
           y0 * prj_mtrx[1][1] + 
           z0 * prj_mtrx[2][1] +
                prj_mtrx[3][1];

 *(vd+2) = x0 * prj_mtrx[0][2] + 
           y0 * prj_mtrx[1][2] + 
           z0 * prj_mtrx[2][2] +
                prj_mtrx[3][2];

}


void project_vector2(float *vd)
{
    float x0 = vd[0];
    float y0 = vd[1];
    float z0 = vd[2];
    float w0 = vd[3];
   
    vd[0]   = x0 * prj_mtrx[0][0] +
              y0 * prj_mtrx[1][0] + 
              z0 * prj_mtrx[2][0];

    vd[1]   = x0 * prj_mtrx[0][1] + 
              y0 * prj_mtrx[1][1] + 
              z0 * prj_mtrx[2][1];

    vd[2]   = x0 * prj_mtrx[0][2] + 
              y0 * prj_mtrx[1][2] + 
              z0 * prj_mtrx[2][2];

    vd[3]   = x0 * prj_mtrx[0][3] + 
              y0 * prj_mtrx[1][3] + 
              z0 * prj_mtrx[2][3];

    vd[0] += prj_mtrx[3][0];
    vd[1] += prj_mtrx[3][1];
    vd[2] += prj_mtrx[3][2]; // + 01;
    vd[3] += prj_mtrx[3][3];

    vd[0] *= rdp_reg.perspnorm;
    vd[1] *= rdp_reg.perspnorm;
    vd[2] *= rdp_reg.perspnorm;
    vd[3] *= rdp_reg.perspnorm;
}

void cmbtrans_vector(float *vd)
{
    float x0 = vd[0];
    float y0 = vd[1];
    float z0 = vd[2];
    float w0 = vd[3];
   
    vd[0]   = x0 * cmb_mtrx[0][0] +
              y0 * cmb_mtrx[1][0] + 
              z0 * cmb_mtrx[2][0];

    vd[1]   = x0 * cmb_mtrx[0][1] + 
              y0 * cmb_mtrx[1][1] + 
              z0 * cmb_mtrx[2][1];

    vd[2]   = x0 * cmb_mtrx[0][2] + 
              y0 * cmb_mtrx[1][2] + 
              z0 * cmb_mtrx[2][2];

    vd[3]   = x0 * cmb_mtrx[0][3] + 
              y0 * cmb_mtrx[1][3] + 
              z0 * cmb_mtrx[2][3];

    vd[0] += cmb_mtrx[3][0];
    vd[1] += cmb_mtrx[3][1];
    vd[2] += cmb_mtrx[3][2]; // + 01;
    vd[3] += cmb_mtrx[3][3];
/*
    vd[0] *= rdp_reg.perspnorm;
    vd[1] *= rdp_reg.perspnorm;
    vd[2] *= rdp_reg.perspnorm;
    vd[3] *= rdp_reg.perspnorm;
*/
}

void cmbtrans_normal(float *vd)
{
    float x0 = vd[0];
    float y0 = vd[1];
    float z0 = vd[2];
   
    vd[0]   = x0 * cmb_mtrx[0][0] +
              y0 * cmb_mtrx[1][0] + 
              z0 * cmb_mtrx[2][0];

    vd[1]   = x0 * cmb_mtrx[0][1] + 
              y0 * cmb_mtrx[1][1] + 
              z0 * cmb_mtrx[2][1];

    vd[2]   = x0 * cmb_mtrx[0][2] + 
              y0 * cmb_mtrx[1][2] + 
              z0 * cmb_mtrx[2][2];

}

void transform_normal(float * vd)
{
    float x = vd[0];
    float y = vd[1];
    float z = vd[2];

//#define _USEINVMATRIX_

#ifndef _USEINVMATRIX_
    *vd     = x * m_stack[m_idx][0][0] + 
              y * m_stack[m_idx][1][0] + 
              z * m_stack[m_idx][2][0];

    *(vd+1) = x * m_stack[m_idx][0][1] + 
              y * m_stack[m_idx][1][1] + 
              z * m_stack[m_idx][2][1];

    *(vd+2) = x * m_stack[m_idx][0][2] + 
              y * m_stack[m_idx][1][2] + 
              z * m_stack[m_idx][2][2];
#else
 // inverse matrix
    *vd     = x * m_stack[m_idx][0][0] + 
              y * m_stack[m_idx][0][1] + 
              z * m_stack[m_idx][0][2];

    *(vd+1) = x * m_stack[m_idx][1][0] + 
              y * m_stack[m_idx][1][1] + 
              z * m_stack[m_idx][1][2];

    *(vd+2) = x * m_stack[m_idx][2][0] + 
              y * m_stack[m_idx][2][1] + 
              z * m_stack[m_idx][2][2];
#endif
    NormalizeVector(vd);
}


void mult_rmatrix(float *m,float r[3][3])
{
    int i,j;float a[3][3];float b[3][3];
    float *ap, *bp, *mp;

    ap=&a[0][0];
    bp=&b[0][0];
    mp=(float*)r;

    for(i=0;i<9;i++) {*ap++=*m++;*bp++=*mp++;}

    for(j=0;j<3;j++)
        for(i=0;i<3;i++)
            r[i][j]=a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j];
}

void reflect_vector(float *v,float *n,float *r)
{
    float x0 = v[0];
    float y0 = v[1];
    float z0 = v[2];
    float s,c,l;

    float reflect_mat[3][3] = {{1,0,0},{0,1,0},{0,0,1}};

    float rot_x[3][3] = {{   1,    0,    0}
                        ,{   0, n[2], n[1]}
                        ,{   0,-n[1], n[2]}};

    float rot_y[3][3] = {{n[0],    0,-n[2]}
                        ,{   0,    1,    0}
                        ,{n[2],    0, n[0]}};

    float rot_z[3][3] = {{ n[0], n[1], 0} 
                        ,{-n[1], n[0], 0}
                        ,{    0,    0, 1}};

    l = (float)sqrt(n[1]*n[1] + n[2]*n[2]);
    if (l > 0)
    {
        s = n[1] / l;
        c = n[2] / l;
    }
    else
    {
        s = 0;
        c = 1;
    }

    rot_x[1][1] = c;
    rot_x[2][2] = c;
    rot_x[1][2] = s;
    rot_x[2][1] = -s;

    l = (float)sqrt(n[2]*n[2] + n[0]*n[0]);
    if (l > 0)
    {
        s = n[2] / l;
        c = n[0] / l;
    }
    else
    {
        s = 0;
        c = 1;
    }

    rot_y[0][0] = c;
    rot_y[2][2] = c;
    rot_y[0][2] = -s;
    rot_y[2][0] = s;

    l = (float)sqrt(n[1]*n[1] + n[0]*n[0]);
    if (l > 0)
    {
        s = n[1] / l;
        c = n[0] / l;
    }
    else
    {
        s = 0;
        c = 1;
    }

    rot_z[0][0] = c;
    rot_z[1][1] = c;
    rot_z[0][1] = s;
    rot_z[1][0] = -s;

    mult_rmatrix((float*)rot_x,reflect_mat);
    mult_rmatrix((float*)rot_y,reflect_mat);
//   mult_rmatrix(rot_z,reflect_mat);

    *r =     x0 * reflect_mat[0][0] + 
             y0 * reflect_mat[1][0] + 
             z0 * reflect_mat[2][0];

    *(r+1) = x0 * reflect_mat[0][1] + 
             y0 * reflect_mat[1][1] + 
             z0 * reflect_mat[2][1];

    *(r+2) = x0 * reflect_mat[0][2] + 
             y0 * reflect_mat[1][2] + 
             z0 * reflect_mat[2][2];

    x0 = r[0];
    y0 = r[1];
    z0 = -r[2];

    *r =     x0 * reflect_mat[0][0] +
             y0 * reflect_mat[0][1] +
             z0 * reflect_mat[0][2];

    *(r+1) = x0 * reflect_mat[1][0] +
             y0 * reflect_mat[1][1] +
             z0 * reflect_mat[1][2];

    *(r+2) = x0 * reflect_mat[2][0] +
             y0 * reflect_mat[2][1] +
             z0 * reflect_mat[2][2];

}

float VectorLength(float *v)
{
    return ((float)sqrt((double)( v[0] * v[0]
                                + v[1] * v[1] +
                                  v[2] * v[2])) );
}
/////////////////////////////////////////////////////////

void push_matrix(void)
{
    int i;float * mp1, *mp2;

    if(m_idx==MAXSTACK-1) 
    {
#ifdef _DEBUGMODE_
        auxprintf("FULL\n");
#endif
        return;
    }

    refresh_matrix = TRUE;

    mp1=&m_stack[m_idx][0][0];
    m_idx++;
    mp2=&m_stack[m_idx][0][0];

    for(i=0;i<16;i++) *mp2++=*mp1++;
}

/////////////////////////////////////////////////////////

void pop_matrix(void)
{
    if(m_idx) m_idx--;
    refresh_matrix = TRUE;
}

/////////////////////////////////////////////////////////

void get_matrix(float * m)
{
    float * mp=&m_stack[m_idx][0][0];
    int i;

    for(i=0;i<16;i++) *m++=*mp++;
}

/////////////////////////////////////////////////////////

void load_matrix(float * m)
{
    float * mp=&m_stack[m_idx][0][0];
    int i;

    refresh_matrix = TRUE;

    for(i=0;i<16;i++) *mp++=*m++;
}

/////////////////////////////////////////////////////////

void JFG_MultMtrx(float m1[4][4], float m2[4][4])
{
    int i,j;
    float a[4][4];
    float b[4][4];//float m[4][4];
    float *ap, *bp, *mp1, *mp2;

    ap=&a[0][0];
    bp=&b[0][0];
    mp1=&m1[0][0];
    mp2=&m2[0][0];

    for(i=0;i<16;i++) {*ap++=*mp1++;*bp++=*mp2++;}

    for(j=0;j<4;j++)
        for(i=0;i<4;i++)
            m1[i][j]=a[i][0] * b[0][j] + a[i][1] * b[1][j] +
                     a[i][2] * b[2][j] + a[i][3] * b[3][j];

//  for(i=0;i<16;i++) {*m1++=*mp++;}
}


/////////////////////////////////////////////////////////

void mult_matrix(float * m)
{
    int i,j;float a[4][4];float b[4][4];
    float *ap, *bp, *mp;

    refresh_matrix = TRUE;

    ap=&a[0][0];
    bp=&b[0][0];
    mp=&m_stack[m_idx][0][0];

    for(i=0;i<16;i++) {*ap++=*m++;*bp++=*mp++;}

    for(j=0;j<4;j++)
        for(i=0;i<4;i++)
            m_stack[m_idx][i][j]=a[i][0] * b[0][j] + a[i][1] * b[1][j] +
                                 a[i][2] * b[2][j] + a[i][3] * b[3][j];
}

/////////////////////////////////////////////////////////

void update_cmbmatrix()
{
    int i,j;float a[4][4];float b[4][4];
    float *ap, *bp, *mp, *pp;

    ap=&a[0][0];
    bp=&b[0][0];
    mp=&m_stack[m_idx][0][0];
    pp=&prj_mtrx[0][0];

    for(i=0;i<16;i++)
    {
        *ap++=*mp++;
        *bp++=*pp++;
    }

    for(j=0;j<4;j++)
        for(i=0;i<4;i++)
            cmb_mtrx[i][j]= a[i][0] * b[0][j] + a[i][1] * b[1][j] +
                            a[i][2] * b[2][j] + a[i][3] * b[3][j];

    refresh_matrix = FALSE;
}

/////////////////////////////////////////////////////////

void load_prj_matrix(float * m)
{
    float * mp=&prj_mtrx[0][0];
    int i;
    
    refresh_matrix = TRUE;

    for(i=0;i<16;i++) *mp++=*m++;
}

/////////////////////////////////////////////////////////

void mult_prj_matrix(float * m)
{
    int i,j;float a[4][4];float b[4][4];
    float *ap, *bp, *mp;
    
    refresh_matrix = TRUE;

    ap=&a[0][0];
    bp=&b[0][0];
    mp=&prj_mtrx[0][0];

    for(i=0;i<16;i++) {*ap++=*m++;*bp++=*mp++;}

    for(j=0;j<4;j++)
        for(i=0;i<4;i++)
            prj_mtrx[i][j]=a[i][0] * b[0][j] + a[i][1] * b[1][j] +
                           a[i][2] * b[2][j] + a[i][3] * b[3][j];
}

/////////////////////////////////////////////////////////

void push_load_matrix(float * m)
{
    int i;float * mp;

    if(m_idx==MAXSTACK-1)
    {
#ifdef _DEBUGMODE_
        auxprintf("FULL\n");
#endif
        return;
    }
    
    refresh_matrix = TRUE;

    m_idx++;

    mp=&m_stack[m_idx][0][0];

    for(i=0;i<16;i++) *mp++=*m++;
}

/////////////////////////////////////////////////////////

void push_mult_matrix(float * m)
{
    int i,j;float a[4][4];float b[4][4];
    float *ap, *bp, *mp;

    if(m_idx==MAXSTACK-1)
    {
#ifdef _DEBUGMODE_
        auxprintf("FULL\n");
        return;
#endif
    }

    refresh_matrix = TRUE;

    ap=&a[0][0];
    bp=&b[0][0];
    mp=&m_stack[m_idx++][0][0];

    for(i=0;i<16;i++) {*ap++=*m++;*bp++=*mp++;}

    for(j=0;j<4;j++)
        for(i=0;i<4;i++)
            m_stack[m_idx][i][j]=a[i][0] * b[0][j] + a[i][1] * b[1][j] +
                                 a[i][2] * b[2][j] + a[i][3] * b[3][j];
}

/////////////////////////////////////////////////////////

