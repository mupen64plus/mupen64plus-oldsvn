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

//****************************************************************
//
// Glide64 - Glide Plugin for Nintendo 64 emulators (tested mostly with Project64)
// Project started on December 29th, 2001
//
// Rules & Instructions
// v1.01
//
// To modify Glide64:
// * Write your name and (optional)email, commented by your work, so I know who did it, and so that you can find which parts you modified when it comes time to send it to me.
// * Do NOT send me the whole project or file that you modified.  Take out your modified code sections, and tell me where to put them.  If people sent the whole thing, I would have many different versions, but no idea how to combine them all.
//
// To add new combine modes:
// * You MUST include an example of something that uses it
// * You MUST write down the names of the GCCMUX_ things
// (see combine.cpp for details)
//
// You may create, modify, steal, use in another plugin, or do whatever you want with this code, but you MUST credit people for their work, and not claim it as your own.
// Neither I, nor anyone who is working with me, take ANY responsibility for your actions or if this program causes harm to anything, including you and your computer, in any way, physically or mentally.
//
// Official Glide64 development channel: #Glide64 on DALnet
//
// Original author: Dave2001 (aka. CM256 or CodeMaster256), Dave2999@hotmail.com
// Other authors listed by the code that they submitted.
//
//****************************************************************

void util_init ();
void clip_z (VERTEX v[3]);
void clip_tri ();

BOOL cull_tri (VERTEX **v);
void DrawTri (VERTEX **v);
void add_tri (VERTEX *v, int n);

void update ();
void TexCache (int tmu, int tileoff);