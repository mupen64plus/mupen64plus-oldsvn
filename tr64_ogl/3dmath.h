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

#ifndef __3DMATH

#define __3DMATH

#include "tr_windows.h"
#include <math.h>

typedef float MATRIX3[9];
typedef float MATRIX4[16];
typedef float VFLOAT;
typedef float VECTOR[4];

extern void PrintMatrix4(MATRIX4 Matrix);
extern void PrintVector(VECTOR vec);
extern void PrintFloat(float f);

extern void m4_submat( MATRIX4 mr, MATRIX3 mb, int i, int j );
extern VFLOAT m4_det( MATRIX4 mr );
extern int m4_inverse( MATRIX4 mr, MATRIX4 ma );
extern int m3_inverse( MATRIX4 mr, MATRIX4 ma );
extern VFLOAT m3_det( MATRIX3 mat );

extern void TranformVector(float *src, float *dst, MATRIX4 mat);
extern void transform_vector2(float * vd,float x0,float y0,float z0,float w0);
extern void project_vector(float * vd,float x0,float y0,float z0);
extern void NormalizeVector(float *v);

extern float DotProduct(float *v1, float *v2);
extern void CrossProduct(float *v1, float *v2, float *v3);
extern BOOL compare_matrix4(MATRIX4 m1, MATRIX4 m2);

extern float VectorLength(float *v);
extern void transform_vector(float * vd,float x0,float y0,float z0);
extern void transform_normal(float * vd);
extern void project_vector(float * vd,float x0,float y0,float z0);
extern void project_vector2(float *vd);
extern void push_matrix(void);
extern void pop_matrix(void);
extern void load_matrix(float * m);
extern void JFG_MultMtrx(float m1[4][4], float m2[4][4]);
extern void mult_matrix(float * m);
extern void load_prj_matrix(float * m);
extern void mult_prj_matrix(float * m);
extern void get_matrix(float * m);
extern void push_mult_matrix(float * m);
extern void push_load_matrix(float * m);
extern void update_cmbmatrix();
extern void cmbtrans_vector(float *vd);
extern void cmbtrans_normal(float *vd);
extern void InversTranformVector(float *src, float *dst);

extern void reflect_vector(float *v,float *n,float *r);

extern void calculate_light_vectors();
extern void math_lighting(t_vtx_tn *vtx, float color[4]);
extern void math_lightingMM(t_vtx *vtx, float color[4]);

extern BOOL refresh_lights;
extern BOOL refresh_matrix;
extern float cmb_mtrx[4][4];
extern float m_stack[16][4][4];
extern float prj_mtrx[4][4];
extern unsigned short m_idx;

#endif
