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

void rsp_texture();
/* rdp graphics instruction function pointer */
static rdp_instr *fast_gfx_instruction;
static rdp_instr gfx_instruction[][256] =
{
    {
/** ucode00 - // 00-3f
        SuperMario64
        Demos
  **/
        spnoop,                     rsp_uc00_matrix,            rsp_reserved0,              rsp_uc00_movemem,
        rsp_uc00_vertex,            rsp_reserved1,              rsp_uc00_displaylist,       rsp_reserved2,
        rsp_reserved3,              rsp_uc00_sprite2d,          fixme,                      fixme,
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
      /** // 40-7f
      * The next 64 commands are not used (as far as i know).
      **/
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
      /** // 80-bf
      * IMMEDIATE commands:
      * The next 64 commands (just 14 are used) are not executed by the RDP.
      * They are like the commands above except that they do not carry a pointer.
      * All data they need is in the command itself.
      * So no DMA transfer is needed.
      **/
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      rsp_uc00_tri2,              rsp_uc00_rdphalf_cont,       rsp_uc00_rdphalf_2,
        rsp_uc00_rdphalf_1,          rsp_uc00_line3d,             rsp_uc00_cleargeometrymode,  rsp_uc00_setgeometrymode,
        rsp_uc00_enddl,              rsp_uc00_setothermode_l,     rsp_uc00_setothermode_h,     rsp_texture, //rsp_uc00_texture,
        rsp_uc00_moveword,           rsp_uc00_popmatrix,          rsp_uc00_culldl,             rsp_uc00_tri1,
      /** // c0-ff
      * RDP commands:
      * These 64 commands are the real RDP commands.
      * I've never seen the TRI commands. They might also be generated by the RSP.
      * 19992406: Now i understand what's going on with these tri commands!!!
      *           With the RSP commands you can set various stuff in dmem. If a
      *           RSP TRI command is called then it looks up that data and calls
      *           the correct RDP tri command - maybe commands!
      *           In HLE emulation you NEVER see this commands.
      *           In "normal" RSP emulation these commands might be used eccessive!
      **/
        rdp_noop,               fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        rdp_trifill,            rdp_trifillz,           rdp_tritxtr,            rdp_tritxtrz,
        rdp_trishade,           rdp_trishadez,          rdp_trishadetxtr,       rdp_trishadetxtrz,
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        rdp_texrect,            rdp_texrectflip,        rdp_loadsync,           rdp_pipesync,
        rdp_tilesync,           rdp_fullsync,           rdp_setkeygb,           rdp_setkeyr,
        rdp_setconvert,         rdp_setscissor,         rdp_setprimdepth,       rdp_setothermode,
        rdp_loadtlut,           fixme,                  rdp_settilesize,        rdp_loadblock,
        rdp_loadtile,           rdp_settile,            rdp_fillrect,           rdp_setfillcolor,
        rdp_setfogcolor,        rdp_setblendcolor,      rdp_setprimcolor,       rdp_setenvcolor,
        rdp_setcombine,         rdp_settextureimage,    rdp_setdepthimage,      rdp_setcolorimage
    },
      

/** ucode01 - 
        WaverRacer Jap
        MarioKart
        StarFox
  **/
      {
        spnoop,                     rsp_uc01_matrix,            rsp_reserved0,              rsp_uc01_movemem,
        rsp_uc01_vertex,            rsp_reserved1,              rsp_uc01_displaylist,       rsp_reserved2,
        rsp_reserved3,              rsp_uc01_sprite2d,          fixme,                      fixme,
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
      /**
      * The next 64 commands are not used (as far as i know).
      **/
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
      /**
      * IMMEDIATE commands:
      * The next 64 commands (just 14 are used) are not executed by the RDP.
      * They are like the commands above except that they do not carry a pointer.
      * All data they need is in the command itself.
      * So no DMA transfer is needed.
      **/
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      rsp_uc01_tri2,              rsp_uc01_rdphalf_cont,      rsp_uc01_rdphalf_2,
        rsp_uc01_rdphalf_1,         rsp_uc01_line3d,            rsp_uc01_cleargeometrymode, rsp_uc01_setgeometrymode,
        rsp_uc01_enddl,             rsp_uc01_setothermode_l,    rsp_uc01_setothermode_h,    rsp_texture, //rsp_uc01_texture,
        rsp_uc01_moveword,          rsp_uc01_popmatrix,         rsp_uc01_culldl,            rsp_uc01_tri1,
      /**
      * RDP commands:
      * These 64 commands are the real RDP commands.
      * I've never seen the TRI commands. They might also be generated by the RSP.
      * 19992406: Now i understand what's going on with these tri commands!!!
      *           With the RSP commands you can set various stuff in dmem. If a
      *           RSP TRI command is called then it looks up that data and calls
      *           the correct RDP tri command - maybe commands!
      *           In HLE emulation you NEVER see this commands.
      *           In "normal" RSP emulation these commands might be used eccessive!
      **/
        rdp_noop,               fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        rdp_trifill,            rdp_trifillz,           rdp_tritxtr,            rdp_tritxtrz,
        rdp_trishade,           rdp_trishadez,          rdp_trishadetxtr,       rdp_trishadetxtrz,
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        rdp_texrect,            rdp_texrectflip,        rdp_loadsync,           rdp_pipesync,
        rdp_tilesync,           rdp_fullsync,           rdp_setkeygb,           rdp_setkeyr,
        rdp_setconvert,         rdp_setscissor,         rdp_setprimdepth,       rdp_setothermode,
        rdp_loadtlut,           fixme,                  rdp_settilesize,        rdp_loadblock,
        rdp_loadtile,           rdp_settile,            rdp_fillrect,           rdp_setfillcolor,
        rdp_setfogcolor,        rdp_setblendcolor,      rdp_setprimcolor,       rdp_setenvcolor,
        rdp_setcombine,         rdp_settextureimage,    rdp_setdepthimage,      rdp_setcolorimage

    },
/** ucode02 - 
        WaverRacer Ntsc/Pal
        Chess3d
  **/
      {
        spnoop,                     rsp_uc02_matrix,             rsp_reserved0,              rsp_uc02_movemem,
        rsp_uc02_vertex,             rsp_reserved1,              rsp_uc02_displaylist,        rsp_reserved2,
        rsp_reserved3,              rsp_uc02_sprite2d,           fixme,                      fixme,
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
      /**
      * The next 64 commands are not used (as far as i know).
      **/
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
      /**
      * IMMEDIATE commands:
      * The next 64 commands (just 14 are used) are not executed by the RDP.
      * They are like the commands above except that they do not carry a pointer.
      * All data they need is in the command itself.
      * So no DMA transfer is needed.
      **/
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      rsp_uc02_tri2,              rsp_uc02_rdphalf_cont,      rsp_uc02_rdphalf_2,
        rsp_uc02_rdphalf_1,         rsp_uc02_quad,              rsp_uc02_cleargeometrymode, rsp_uc02_setgeometrymode,
        rsp_uc02_enddl,             rsp_uc02_setothermode_l,    rsp_uc02_setothermode_h,    rsp_texture, //rsp_uc02_texture,
        rsp_uc02_moveword,          rsp_uc02_popmatrix,         rsp_uc02_culldl,            rsp_uc02_tri1,
      /**
      * RDP commands:
      * These 64 commands are the real RDP commands.
      * I've never seen the TRI commands. They might also be generated by the RSP.
      * 19992406: Now i understand what's going on with these tri commands!!!
      *           With the RSP commands you can set various stuff in dmem. If a
      *           RSP TRI command is called then it looks up that data and calls
      *           the correct RDP tri command - maybe commands!
      *           In HLE emulation you NEVER see this commands.
      *           In "normal" RSP emulation these commands might be used eccessive!
      **/
        rdp_noop,               fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        rdp_trifill,            rdp_trifillz,           rdp_tritxtr,            rdp_tritxtrz,
        rdp_trishade,           rdp_trishadez,          rdp_trishadetxtr,       rdp_trishadetxtrz,
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        rdp_texrect,            rdp_texrectflip,        rdp_loadsync,           rdp_pipesync,
        rdp_tilesync,           rdp_fullsync,           rdp_setkeygb,           rdp_setkeyr,
        rdp_setconvert,         rdp_setscissor,         rdp_setprimdepth,       rdp_setothermode,
        rdp_loadtlut,           fixme,                  rdp_settilesize,        rdp_loadblock,
        rdp_loadtile,           rdp_settile,            rdp_fillrect,           rdp_setfillcolor,
        rdp_setfogcolor,        rdp_setblendcolor,      rdp_setprimcolor,       rdp_setenvcolor,
        rdp_setcombine,         rdp_settextureimage,    rdp_setdepthimage,      rdp_setcolorimage

    },
/** ucode03 - 
        Mortal Kombat 4
  **/
      {
//0xf0
        spnoop,                     rsp_uc03_matrix,            rsp_reserved0,              rsp_uc03_movemem,
        rsp_uc03_vertex,            rsp_reserved1,              rsp_uc03_displaylist,       rsp_reserved2,
        rsp_reserved3,              rsp_uc03_sprite2d,          fixme,                      fixme,
        fixme,                      fixme,                      fixme,                      fixme,        
//0xf0
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0xf0
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0xf0
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
      /**
      * The next 64 commands are not used (as far as i know).
      **/
//0xf0
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0xf0
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0xf0
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0xf0
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
      /**
      * IMMEDIATE commands:
      * The next 64 commands (just 14 are used) are not executed by the RDP.
      * They are like the commands above except that they do not carry a pointer.
      * All data they need is in the command itself.
      * So no DMA transfer is needed.
      **/
//0x80
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0x90
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0xA0
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0xB0
        fixme,                      rsp_uc03_tri2,              rsp_uc03_rdphalf_cont,      rsp_uc03_rdphalf_2,
        rsp_uc03_rdphalf_1,         rsp_uc03_line3d,            rsp_uc03_cleargeometrymode, rsp_uc03_setgeometrymode,
        rsp_uc03_enddl,             rsp_uc03_setothermode_l,    rsp_uc03_setothermode_h,    rsp_texture, //rsp_uc03_texture,
        rsp_uc03_moveword,          rsp_uc03_popmatrix,         rsp_uc03_culldl,            rsp_uc03_tri1,
      /**
      * RDP commands:
      * These 64 commands are the real RDP commands.
      * I've never seen the TRI commands. They might also be generated by the RSP.
      * 19992406: Now i understand what's going on with these tri commands!!!
      *           With the RSP commands you can set various stuff in dmem. If a
      *           RSP TRI command is called then it looks up that data and calls
      *           the correct RDP tri command - maybe commands!
      *           In HLE emulation you NEVER see this commands.
      *           In "normal" RSP emulation these commands might be used eccessive!
      **/
//0xC0
        rdp_noop,               fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        rdp_trifill,            rdp_trifillz,           rdp_tritxtr,            rdp_tritxtrz,
        rdp_trishade,           rdp_trishadez,          rdp_trishadetxtr,       rdp_trishadetxtrz,
//0xD0
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
//0xE0
        fixme,                  fixme,                  fixme,                  fixme,    
        rdp_texrect,            rdp_texrectflip,        rdp_loadsync,           rdp_pipesync,
        rdp_tilesync,           rdp_fullsync,           rdp_setkeygb,           rdp_setkeyr,
        rdp_setconvert,         rdp_setscissor,         rdp_setprimdepth,       rsp_uc03_setothermode,
//0xF0
        rdp_loadtlut,           fixme,                  rdp_settilesize,        rdp_loadblock,
        rdp_loadtile,           rdp_settile,            rdp_fillrect,           rdp_setfillcolor,
        rdp_setfogcolor,        rdp_setblendcolor,      rdp_setprimcolor,       rdp_setenvcolor,
        rdp_setcombine,         rdp_settextureimage,    rdp_setdepthimage,      rdp_setcolorimage

    },
/** ucode04 - 
        Zelda
  **/
      {
//0x00
        spnoop,                     rsp_uc04_vertex,            rsp_uc04_modifyvertex,      rsp_uc04_culldl,
        rsp_uc04_branchz,           rsp_uc04_tri1,              rsp_uc04_tri2,              rsp_uc04_quad,
        rsp_uc04_line3d,            fixme,                      rsp_uc04_fullscreen,        fixme,
        fixme,                      fixme,                      fixme,                      fixme,        
//0x10
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0x20
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0x30
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
      /**
      * The next 64 commands are not used (as far as i know).
      **/
//0x40
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0x50
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0x60
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0x70
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
      /**
      * IMMEDIATE commands:
      * The next 64 commands (just 14 are used) are not executed by the RDP.
      * They are like the commands above except that they do not carry a pointer.
      * All data they need is in the command itself.
      * So no DMA transfer is needed.
      **/
//0x80
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0x90
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0xA0
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0xB0
        fixme,                      fixme,                      fixme,                      fixme,
        fixme,                      fixme,                      fixme,                      fixme,
        fixme,                      fixme,                      fixme,                      fixme,
        fixme,                      fixme,                      fixme,                      fixme,
      /**
      * RDP commands:
      * These 64 commands are the real RDP commands.
      * I've never seen the TRI commands. They might also be generated by the RSP.
      * 19992406: Now i understand what's going on with these tri commands!!!
      *           With the RSP commands you can set various stuff in dmem. If a
      *           RSP TRI command is called then it looks up that data and calls
      *           the correct RDP tri command - maybe commands!
      *           In HLE emulation you NEVER see this commands.
      *           In "normal" RSP emulation these commands might be used eccessive!
      **/
//0xC0
        rdp_noop,               fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        rdp_trifill,            rdp_trifillz,           rdp_tritxtr,            rdp_tritxtrz,
        rdp_trishade,           rdp_trishadez,          rdp_trishadetxtr,       rdp_trishadetxtrz,
//0xD0
        fixme,                  fixme,                  fixme,                  rsp_uc04_special_1,    
        rsp_uc04_special_2,     rsp_uc04_special_3,      rsp_uc04_dma_io,       rsp_uc04_texture,// rsp_texture
        rsp_uc04_popmatrix,     rsp_uc04_setgeometrymode,rsp_uc04_matrix,       rsp_uc04_moveword,    
        rsp_uc04_movemem,       rsp_uc04_load_ucode,      rsp_uc04_displaylist,   rsp_uc04_enddl,    
//0xE0
        rsp_uc04_noop,          rsp_uc04_rdphalf_1,     rsp_uc04_setothermode_l, rsp_uc04_setothermode_h, 
        rdp_texrect,            rdp_texrectflip,        rdp_loadsync,           rdp_pipesync,
        rdp_tilesync,           rdp_fullsync,           rdp_setkeygb,           rdp_setkeyr,
        rdp_setconvert,         rdp_setscissor,         rdp_setprimdepth,       rdp_setothermode,
//0xF0
        rdp_loadtlut,           fixme,                  rdp_settilesize,        rdp_loadblock,
        rdp_loadtile,           rdp_settile,            rdp_fillrect,           rdp_setfillcolor,
        rdp_setfogcolor,        rdp_setblendcolor,      rdp_setprimcolor,       rdp_setenvcolor,
        rdp_setcombine,         rdp_settextureimage,    rdp_setdepthimage,      rdp_setcolorimage

    },
/** ucode05 - 
        Dark Project 
        Golden Eye
  **/
      {
// 0x00
        spnoop,                     rsp_uc05_matrix,            rsp_reserved0,              rsp_uc05_movemem,
        rsp_uc05_vertex,            rsp_reserved1,              rsp_uc05_displaylist,       rsp_reserved2,
        rsp_reserved3,              rsp_uc05_sprite2d,          fixme,                      fixme,
        fixme,                      fixme,                      fixme,                      fixme,        
// 0x10
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
// 0x20
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
// 0x30
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
      /**
      * The next 64 commands are not used (as far as i know).
      **/
// 0x40
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
// 0x50
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
// 0x60
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
// 0x70
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
      /**
      * IMMEDIATE commands:
      * The next 64 commands (just 14 are used) are not executed by the RDP.
      * They are like the commands above except that they do not carry a pointer.
      * All data they need is in the command itself.
      * So no DMA transfer is needed.
      **/
// 0x80
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
// 0x90
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
// 0xA0
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
// 0xB0
        fixme,                      rsp_uc05_tri4,               rsp_uc05_rdphalf_cont,       rsp_uc05_rdphalf_2,
        rsp_uc05_rdphalf_1,         rsp_uc05_line3d,             rsp_uc05_cleargeometrymode,  rsp_uc05_setgeometrymode,
        rsp_uc05_enddl,             rsp_uc05_setothermode_l,     rsp_uc05_setothermode_h,     rsp_texture, //rsp_uc05_texture,
        rsp_uc05_moveword,          rsp_uc05_popmatrix,          rsp_uc05_culldl,             rsp_uc05_tri1,
      /**
      * RDP commands:
      * These 64 commands are the real RDP commands.
      * I've never seen the TRI commands. They might also be generated by the RSP.
      * 19992406: Now i understand what's going on with these tri commands!!!
      *           With the RSP commands you can set various stuff in dmem. If a
      *           RSP TRI command is called then it looks up that data and calls
      *           the correct RDP tri command - maybe commands!
      *           In HLE emulation you NEVER see this commands.
      *           In "normal" RSP emulation these commands might be used eccessive!
      **/
// 0xC0
        rdp_noop,               fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        rdp_trifill,            rdp_trifillz,           rdp_tritxtr,            rdp_tritxtrz,
        rdp_trishade,           rdp_trishadez,          rdp_trishadetxtr,       rdp_trishadetxtrz,
// 0xD0
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
// 0xE0
        fixme,                  fixme,                  fixme,                  fixme,    
        rdp_texrect,            rdp_texrectflip,        rdp_loadsync,           rdp_pipesync,
        rdp_tilesync,           rdp_fullsync,           rdp_setkeygb,           rdp_setkeyr,
        rdp_setconvert,         rdp_setscissor,         rdp_setprimdepth,       rdp_setothermode,
// 0xF0
        rdp_loadtlut,           fixme,                  rdp_settilesize,        rdp_loadblock,
        rdp_loadtile,           rdp_settile,            rdp_fillrect,           rdp_setfillcolor,
        rdp_setfogcolor,        rdp_setblendcolor,      rdp_setprimcolor,       rdp_setenvcolor,
        rdp_setcombine,         rdp_settextureimage,    rdp_setdepthimage,      rdp_setcolorimage

    },

//  ucode06 -
//      Diddy Kong Racing
//      Jet Force Gemini
      {
// 0x00
        spnoop,                     rsp_uc06_matrix,            rsp_reserved0,              rsp_uc06_movemem,
        rsp_uc06_vertex,            rsp_uc06_tri4,              rsp_uc06_displaylist,       rsp_uc06_dlinmem,
        rsp_reserved3,              rsp_uc06_sprite2d,          fixme,                      fixme,
        fixme,                      fixme,                      fixme,                      fixme,         
// 0x10
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
// 0x20
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
// 0x30
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
      /**
      * The next 64 commands are not used (as far as i know).
      **/
// 0x40
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
// 0x50
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
// 0x60
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
// 0x70
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
      /**
      * IMMEDIATE commands:
      * The next 64 commands (just 14 are used) are not executed by the RDP.
      * They are like the commands above except that they do not carry a pointer.
      * All data they need is in the command itself.
      * So no DMA transfer is needed.
      **/
// 0x80
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
// 0x90
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
// 0xA0
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
// 0xB0
        fixme,                      rsp_uc06_tri4,              rsp_uc06_rdphalf_cont,      rsp_uc06_rdphalf_2,
        rsp_uc06_rdphalf_1,         rsp_uc06_line3d,            rsp_uc06_cleargeometrymode, rsp_uc06_setgeometrymode,
        rsp_uc06_enddl,             rsp_uc06_setothermode_l,    rsp_uc06_setothermode_h,    rsp_texture, //rsp_uc06_texture,
        rsp_uc06_moveword,          rsp_uc06_popmatrix,         rsp_uc06_culldl,            rsp_uc06_tri1,
      /**
      * RDP commands:
      * These 64 commands are the real RDP commands.
      * I've never seen the TRI commands. They might also be generated by the RSP.
      * 19992406: Now i understand what's going on with these tri commands!!!
      *           With the RSP commands you can set various stuff in dmem. If a
      *           RSP TRI command is called then it looks up that data and calls
      *           the correct RDP tri command - maybe commands!
      *           In HLE emulation you NEVER see this commands.
      *           In "normal" RSP emulation these commands might be used eccessive!
      **/
// 0xC0
        rdp_noop,               fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        rdp_trifill,            rdp_trifillz,           rdp_tritxtr,            rdp_tritxtrz,
        rdp_trishade,           rdp_trishadez,          rdp_trishadetxtr,       rdp_trishadetxtrz,
// 0xD0
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
// 0xE0
        fixme,                  fixme,                  fixme,                  fixme,    
        rdp_texrect,            rdp_texrectflip,        rdp_loadsync,           rdp_pipesync,
        rdp_tilesync,           rdp_fullsync,           rdp_setkeygb,           rdp_setkeyr,
        rdp_setconvert,         rdp_setscissor,         rdp_setprimdepth,       rdp_setothermode,
// 0xF0
        rdp_loadtlut,           fixme,                  rdp_settilesize,        rdp_loadblock,
        rdp_loadtile,           rdp_settile,            rdp_fillrect,           rdp_setfillcolor,
        rdp_setfogcolor,        rdp_setblendcolor,      rdp_setprimcolor,       rdp_setenvcolor,
        rdp_setcombine,         rdp_settextureimage,    rdp_setdepthimage,      rdp_setcolorimage

    },
/** ucode07 - 
        Conker's Bad Fur Day
  **/
      {
//0x00
        spnoop,                     rsp_uc07_vertex,            rsp_uc07_modifyvertex,      rsp_uc07_culldl,
        rsp_uc07_branchz,           rsp_uc07_tri1,              rsp_uc07_tri2,              rsp_uc07_quad,
        rsp_uc07_line3d,            fixme,                      rsp_uc07_fullscreen,        fixme,
        fixme,                      fixme,                      fixme,                      fixme,        
//0x10

        rsp_uc07_tri4,              rsp_uc07_tri4,              rsp_uc07_tri4,              rsp_uc07_tri4,        
        rsp_uc07_tri4,              rsp_uc07_tri4,              rsp_uc07_tri4,              rsp_uc07_tri4,        
        rsp_uc07_tri4,              rsp_uc07_tri4,              rsp_uc07_tri4,              rsp_uc07_tri4,        
        rsp_uc07_tri4,              rsp_uc07_tri4,              rsp_uc07_tri4,              rsp_uc07_tri4,        
/*
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
*/
//0x20
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0x30
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
      /**
      * The next 64 commands are not used (as far as i know).
      **/
//0x40
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0x50
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0x60
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0x70
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
      /**
      * IMMEDIATE commands:
      * The next 64 commands (just 14 are used) are not executed by the RDP.
      * They are like the commands above except that they do not carry a pointer.
      * All data they need is in the command itself.
      * So no DMA transfer is needed.
      **/
//0x80
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0x90
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0xA0
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
        fixme,                      fixme,                      fixme,                      fixme,        
//0xB0
        fixme,                      fixme,                      fixme,                      fixme,
        fixme,                      fixme,                      fixme,                      fixme,
        fixme,                      fixme,                      fixme,                      fixme,
        fixme,                      fixme,                      fixme,                      fixme,
      /**
      * RDP commands:
      * These 64 commands are the real RDP commands.
      * I've never seen the TRI commands. They might also be generated by the RSP.
      * 19992406: Now i understand what's going on with these tri commands!!!
      *           With the RSP commands you can set various stuff in dmem. If a
      *           RSP TRI command is called then it looks up that data and calls
      *           the correct RDP tri command - maybe commands!
      *           In HLE emulation you NEVER see this commands.
      *           In "normal" RSP emulation these commands might be used eccessive!
      **/
//0xC0
        rdp_noop,               fixme,                  fixme,                  fixme,    
        fixme,                  fixme,                  fixme,                  fixme,    
        rdp_trifill,            rdp_trifillz,           rdp_tritxtr,            rdp_tritxtrz,
        rdp_trishade,           rdp_trishadez,          rdp_trishadetxtr,       rdp_trishadetxtrz,
//0xD0
        fixme,                  fixme,                  fixme,                  rsp_uc07_special_1,    
        rsp_uc07_special_2,     rsp_uc07_special_3,      rsp_uc07_dma_io,       rsp_uc04_texture,    
        rsp_uc07_popmatrix,     rsp_uc07_setgeometrymode,rsp_uc07_matrix,       rsp_uc07_moveword,    
        rsp_uc07_movemem,       rsp_uc07_load_ucode,      rsp_uc07_displaylist, rsp_uc07_enddl,    
//0xE0
        rsp_uc07_noop,          rsp_uc07_rdphalf_1,     rsp_uc07_setothermode_l, rsp_uc07_setothermode_h, 
        rdp_texrect,            rdp_texrectflip,        rdp_loadsync,           rdp_pipesync,
        rdp_tilesync,           rdp_fullsync,           rdp_setkeygb,           rdp_setkeyr,
        rdp_setconvert,         rdp_setscissor,         rdp_setprimdepth,       rdp_setothermode,
//0xF0
        rdp_loadtlut,           fixme,                  rdp_settilesize,        rdp_loadblock,
        rdp_loadtile,           rdp_settile,            rdp_fillrect,           rdp_setfillcolor,
        rdp_setfogcolor,        rdp_setblendcolor,      rdp_setprimcolor,       rdp_setenvcolor,
        rdp_setcombine,         rdp_settextureimage,    rdp_setdepthimage,      rdp_setcolorimage

    }
};

