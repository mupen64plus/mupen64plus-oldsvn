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
#include "type_sizes.h"

#include "wingl.h"

#include "debug.h"
#include "Combine.h"
#include "crc_ucode.h"


//** externals
extern int              *pInterruptMask;
extern unsigned char    *pRDRAM;
extern unsigned char    *pIDMEM;
extern unsigned char    *pVIREG;
extern _u32             *pVIORG;

char output[1024];
_u32 pltmode;
extern _u32 *Palette8;;
extern _u32 Palette8IA[256];
extern _u32 Palette8RGBA[256];
//extern unsigned long Palette4[256];
extern int  Vtidx[256];
extern int Vtcnt;
extern void FlushVisualTriangle(int vn[]);
extern void loadtile(int tile);

#include "rdp.h"
#include "rdp_registers.h"
#include "Combine.h"

// FiRES
extern  int ucode_version;
extern HWND hGraphics;
extern BOOLEAN Draw2d;
#include "rdp_gl.h"
#include "Render.h"
/* define macros - to speed up things and to make things looking nice */

#define MAXSMASK 7
#define MAXTMASK 7

#define SKIPMASK 1
BOOL CamSpyGo = TRUE;
BOOL CamSpyON = FALSE;
_u32 PaletteCRC[17];

_u8 m_tmem[0x1010];

_u32 InvPaletteCRC[17];

t_rdp_reg rdp_reg;
static  _u32 segoffset2addr(_u32 so);   /* calcs the real addr out of seg and offset */
int DList_C = 0;

//** rdp instructions (audio & graphics) 
//** All ucodes 

// Globals 
//#define LstLdAddrSize (32 * 32)
#define LstLdAddrSize (64)
#define LstLdAddrMask (LstLdAddrSize - 1)

LoadCache       tmpLdAddr;
LoadCache       LstLdAddr[LstLdAddrSize];
_u32            LstLdAddNum = 0;

static char *format[]   = { "RGBA", "YUV", "CI", "IA", "I", "?", "?", "?" };
static char *size[]     = { "4bit", "8bit", "16bit", "32bit" };
static char *cm[]       = { "NOMIRROR/WARP(NOCLAMP)", "MIRROR", "CLAMP", "MIRROR&CLAMP" };

static void fixme();
static void spnoop();

static void rdp_noop();
static void rdp_texrect();
static void rdp_texrectflip();
static void rdp_loadsync();
static void rdp_pipesync();
static void rdp_tilesync();
static void rdp_fullsync();
static void rdp_setkeygb();
static void rdp_setkeyr();
static void rdp_setconvert();
static void rdp_setscissor();
static void rdp_setprimdepth();
static void rdp_setothermode();
static void rdp_loadtlut();
static void rdp_settilesize();
static void rdp_loadblock();
static void rdp_loadtile();
static void rdp_settile();
static void rdp_fillrect();
static void rdp_setfillcolor();
static void rdp_setfogcolor();
static void rdp_setblendcolor();
static void rdp_setprimcolor();
static void rdp_setenvcolor();
extern void rdp_setcombine();
static void rdp_settextureimage();
static void rdp_setdepthimage();
static void rdp_setcolorimage();
static void rdp_trifill();
static void rdp_trishade();
static void rdp_tritxtr();
static void rdp_trishadetxtr();
static void rdp_trifillz();
static void rdp_trishadez();
static void rdp_tritxtrz();
static void rdp_trishadetxtrz();
static void rsp_texture();

static void rsp_reserved0();
static void rsp_reserved1();
static void rsp_reserved2();
static void rsp_reserved3();
void DelTexture();


static void MathTextureScales();
void hleGetMatrix(float *dst,_u8 *src);

void exec_gfx();

int swapMode = 0;
int swapmodecnt = 5;
_u32 lastaddr;
BOOL SwapNow;

extern float imgHeight;
extern float imgWidth;
extern _u32 vi_height;
extern _u32 vi_width;
extern BOOL vi_Hires;

//////////////////////////////////////////////////////////////////////////////
// structure is tricky for changing the endians                             //
//////////////////////////////////////////////////////////////////////////////
float UC6_Matrices[8][4][4];


_u8 doReadMemByte(_u32 where)
{
        where ^= 0x03;
        return(*((_u8 *)&pRDRAM[where]));

} // _u8 doReadMemByte(_u32 where) 


_u16 doReadMemHalfWord(_u32 where)
{
        where ^= 0x02;
        return(*((_u16 *)&pRDRAM[where]));

} // _u16 doReadMemHalfWord(_u32 where) 


_u32 doReadMemWord(_u32 where)
{
     return(*((_u32 *)&pRDRAM[where]));
} // _u32 doReadMemWord(_u32 where) 


_u64 doReadMemDoubleWord(_u32 where)
{
        _u64 value;
        value = (*((_u64 *)&pRDRAM[where]));
        return( (value >> 32) | (value << 32) );
} // _u64 doReadMemDoubleWord(_u32 where) 



#include "hle_uc00.h"
#include "hle_uc01.h"
#include "hle_uc02.h"
#include "hle_uc03.h"
#include "hle_uc04.h"
#include "hle_uc05.h"
#include "hle_uc06.h"
#include "hle_uc07.h"
#include "hle_uc.h"             // All Functions-Array


void exec_gfx()
{
    gfx_instruction[ucode_version][rdp_reg.cmd0>>24]();
}
/******************************************************************************\
*                                                                              *
*   RDP (Reality Display Processor) - global routines                          *
*                                                                              *
\******************************************************************************/
static void MathTextureScales()
{
    int i = rdp_reg.tile;
    float TScaleT, TScaleS;
    _u32 twidth,theight;

    twidth=rdp_reg.m_CurTile->Width;

    theight=rdp_reg.m_CurTile->Height;

    //twidth=rdp_reg.m_CurTile->lrs - rdp_reg.m_CurTile->uls + 1;

    //theight=rdp_reg.m_CurTile->lrt - rdp_reg.m_CurTile->ult + 1;

    if(twidth==0) twidth=1;
    if(theight==0) theight=1;

    if(rdp_reg.m_CurTile->shifts<MAXSHIFTS)
        twidth<<=rdp_reg.m_CurTile->shifts;
    else
        twidth>>=(16-rdp_reg.m_CurTile->shifts);

    if(rdp_reg.m_CurTile->shiftt<MAXSHIFTT)
        theight<<=rdp_reg.m_CurTile->shiftt;
    else
        theight>>=(16-rdp_reg.m_CurTile->shiftt);

    TScaleS = rdp_reg.m_CurTile->SScale;
    TScaleT = rdp_reg.m_CurTile->TScale;

    if (rdp_reg.m_CurTile->mirrors)
        TScaleS /= 2;

    if (rdp_reg.m_CurTile->mirrort)
        TScaleT /= 2;


    rdp_reg.texture[i].scale_t = TScaleT;
    rdp_reg.texture[i].scale_s = TScaleS;   

    rdp_reg.texture[i].scale_w = 1.0f / twidth;
    rdp_reg.texture[i].scale_h = 1.0f / theight;    
}

void hleGetMatrix(float *dst,_u8 *src)
{
    float Const=1.f/65536.f;
    _asm
    {
        mov     edi,dword ptr dst
        mov     esi,dword ptr src
        mov     ecx,8
mat_loop:
        push    ecx
        mov     eax,dword ptr [esi]
        mov     ebx,eax
        and     eax,0xffff0000
        and     ebx,0x0ffff
        shl     ebx,16
        mov     ecx,dword ptr [esi+32]
        mov     edx,ecx
        shr     ecx,16
        mov     ax,cx
        mov     bx,dx
        add     esi,4
        mov     dword ptr [edi+4],eax
        mov     dword ptr [edi],ebx
        fild    dword ptr [edi]
        fmul    DWORD PTR Const
        fild    dword ptr [edi+4]
        fmul    DWORD PTR Const
        pop     ecx
        fstp    DWORD PTR [edi]
        fstp    DWORD PTR [edi+4]
        add     edi,8
        loop    mat_loop
    }
}



extern unsigned short m_idx;
extern int ArrayOk;

float tmpMat[4][4] = {{1.0f,0.0f,0.0f,0.0f}
                     ,{0.0f,1.0f,0.0f,0.0f}
                     ,{0.0f,0.0f,1.0f,0.0f}
                     ,{0.0f,0.0f,0.0f,1.0f}};

void rdp_reset()
{
    int idx;

    load_matrix((float*)tmpMat);
    
    ArrayOk = 0;

    for (idx = 0; idx < 16; idx++) rdp_reg.segment[idx] = 0xffffffff;

        rdp_reg.m_CurTile = &rdp_reg.td[0];
        rdp_reg.tile = 0;
        //rdp_reg.m_NexTile = &rdp_reg.td[1];
        
        rdp_reg.pc_i = 0;
        rdp_reg.halt = 1;
        rdp_reg.lights = 1;
        rdp_reg.ambient_light =  rdp_reg.lights;

        rdp_reg.tile = 0;
        rdp_reg.loadtile = 7;
        rdp_reg.td[0].set_by = RDPTD_LOAD_NOT_SET;
        rdp_reg.td[1].set_by = RDPTD_LOAD_NOT_SET;
        rdp_reg.td[2].set_by = RDPTD_LOAD_NOT_SET;
        rdp_reg.td[3].set_by = RDPTD_LOAD_NOT_SET;
        rdp_reg.td[4].set_by = RDPTD_LOAD_NOT_SET;
        rdp_reg.td[5].set_by = RDPTD_LOAD_NOT_SET;
        rdp_reg.td[6].set_by = RDPTD_LOAD_NOT_SET;
        rdp_reg.td[7].set_by = RDPTD_LOAD_NOT_SET;

        Vtcnt = 0;
        m_idx=0;
        rdp_reg.perspnorm = 1.0f;
        ucode_version = -1;

        glMatrixMode(GL_PROJECTION);
        //glPushMatrix();
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        //glPushMatrix();
        glLoadIdentity();

        Render_ClearVisual();
        ModesUsedCnt = 0;
} /* void rdp_reset() */

static _u32 nextcmd;
static int rdpbusy = 0;
static int skipfram = 0;
_u32 lastVIorg = 0;
BOOL res = FALSE;

// prefixed with "Real" because we have "ExecuteDList" exported from 
// driver.h and it wraps this function.
void RealExecuteDList(unsigned __int32 dwAddr)
{
    _u32   a,ta;
    int cnt = 0x1000;
    int tcmd;
    ////glDrawBuffer(GL_AUX0);

    fast_gfx_instruction = gfx_instruction[ucode_version];

    rdp_reg.task = GFX_TASK;
    rdp_reg.pc_i = 0;
    rdp_reg.pc[rdp_reg.pc_i] = dwAddr;
    rdp_reg.halt = 0;
    //rdp_reg.m_CurTile = &rdp_reg.td[0];
    //rdp_reg.tile = 0;
    //rdp_reg.m_NexTile = &rdp_reg.td[1];

    m_idx=0;

    //CamSpyGo = TRUE;
    CamSpyON = FALSE;
    rdp_reg.perspnorm = 1.0f;

    Draw2d = FALSE;

    if (!res)
        res = wglMakeCurrent(WinData.hDC, WinData.hGLRC);

//  if (!res) WGL_Init();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    SwapNow = TRUE;
    rdp_reg.mode_l |= 0x30;
    
    glEnable(GL_DEPTH_TEST);
    skipfram++;

#ifdef LOG_ON
    LOG_TO_FILE("\n****************************************");
    LOG_TO_FILE(  "* NEW DLIST                            *");
    LOG_TO_FILE(  "* address: %08X                    *", dwAddr);
    LOG_TO_FILE(  "* DList-no: % 6i                     *", DList_C);
    LOG_TO_FILE(  "****************************************\n");
#endif

    //Render_ResetViewPort();
    
    if (TRUE)
//  if (skipfram > 83)
//  if ((skipfram & SKIPMASK) == 0)
//  if (0)
    {
        if (swapMode == 0)
        {
            Render_FlushVisualRenderBuffer();
            Render_ClearVisual();
            CamSpyGo = TRUE;
        }
        do
        {
//          a = rdp_reg.pc[rdp_reg.pc_i];
            a = rdp_reg.pc[rdp_reg.pc_i] & 0x007fffff;
            ta = a >> 2;

        //** load next commando
            rdp_reg.cmd0 = ((_u32 *)pRDRAM)[ta + 0];
            rdp_reg.cmd1 = ((_u32 *)pRDRAM)[ta + 1];
            nextcmd      = ((_u32 *)pRDRAM)[ta + 2];
            rdp_reg.cmd2 = ((_u32 *)pRDRAM)[ta + 3];
            rdp_reg.cmd3 = ((_u32 *)pRDRAM)[ta + 5];

        //** point to next instruction 
            rdp_reg.curr = rdp_reg.pc[rdp_reg.pc_i] = (a + 8) & 0x007fffff;   

        //** execute next gfx-instruction
            tcmd = (_u8)(rdp_reg.cmd0>>24);

            switch (ucode_version)
            {
            case 0:
            case 1:
            case 2:
            case 3:
            case 5:
                if (((Vtcnt > 0) && (tcmd != 0xb1) && (tcmd != 0xb5) && (tcmd != 0xbf)))
                {
                    FlushVisualTriangle(Vtidx);
                    Vtcnt = 0;
                }
                break;
            case 4:
                if (((Vtcnt > 0) && (tcmd != 0x05) && (tcmd != 0x06) && (tcmd != 0x07)))
                {
                    FlushVisualTriangle(Vtidx);
                    Vtcnt = 0;
                }
                break;
            case 6:
            case 7:
                if (((Vtcnt > 0) && (tcmd != 0x05) && (tcmd != 0x06) && (tcmd != 0x07) && !(tcmd > 0x0f && tcmd < 0x20)))
                {
                    FlushVisualTriangle(Vtidx);
                    Vtcnt = 0;
                }
                break;
            }

            fast_gfx_instruction[rdp_reg.cmd0>>24]();

        } while(!rdp_reg.halt);

    }
    else //if (((rdp_reg.cmd0>>24) & 0x0ff) == 0xb8)
    {
        //gfx_instruction[ucode_version][rdp_reg.cmd0>>24]();
        rdp_fullsync();
        rdp_reg.halt = 1;
    }

    DList_C++;
} /* rdp_execute_dlist() */




/******************************************************************************\
*                                                                              *
*   HLE (High Level Emulation) - helper routine.                               *
*                                                                              *
\******************************************************************************/

static _u32 segoffset2addr(_u32 so)
{
//        return( (rdp_reg.segment[(so>>24)&0x0f] + (so&0x00ffffff))  & 0x007fffff );
    _u32 seg = (so>>24)&0x0f;

    if (rdp_reg.segment[seg] != 0xffffffff)
        return((rdp_reg.segment[seg] + (so&0x00ffffff))  & 0x007fffff );
    else
        return((rdp_reg.segment[0] + (so&0x00ffffff))  & 0x007fffff );
} /* static _u32 segoffset2addr(_u32 so) */



static void rsp_reserved0()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RSP_RESERVED0 ", ADDR, CMD0, CMD1);
#endif
} /* static void rsp_reserved0() */

static void rsp_reserved1()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RSP_RESERVED1 \n", ADDR, CMD0, CMD1);
#endif
} /* static void rsp_reserved1() */

static void rsp_reserved2()
{
        rdp_reg.ColorInfoAdd = segoffset2addr(rdp_reg.cmd1);
        rdp_reg.ColorMode = (rdp_reg.geometrymode & 0x00020000)? TRUE : FALSE;

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD SETCOLORINFOADD ", ADDR, CMD0, CMD1);
    LOG_TO_FILE("\tColorInfoAdd = %08X", rdp_reg.ColorInfoAdd);
    LOG_TO_FILE("\tColorMode = %08X\n", rdp_reg.ColorMode);
#endif
} /* static void rsp_reserved2() */


static void rsp_reserved3()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RSP_RESERVED3 \n", ADDR, CMD0, CMD1);
#endif        

} /* static void rsp_reserved3() */


/******************************************************************************\
*                                                                              *
*   RDP (Reality Display Processor) - static routines (graphics and audio)     *
*                                                                              *
\******************************************************************************/



static void fixme()
{
//  _u32 add = segoffset2addr(rdp_reg.cmd1);

//  MessageBox(hGraphics, "Problem with UCode ...\ntry another one", "fixme", MB_OK);
/*      sprintf(output,"uCode = %i\ncmd0 = %8x\ncmd1 = %8x\ncmd2 = %8x\ncmd3 = %8x\nadd  = %8x",ucode_version,rdp_reg.cmd0,rdp_reg.cmd1,rdp_reg.cmd2,rdp_reg.cmd3,add);
        MessageBox(hGraphics, output, "fixme", MB_OK);
*/
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD FIXME %08X\n", ADDR, CMD0, CMD1,rdp_reg.cmd0>>24);
#endif
} // static void fixme() 





static void spnoop()
{
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X SPNOOP\n", ADDR, CMD0, CMD1);
#endif
} /* static void spnoop() */





/******************************************************************************\
*                                                                              *
*   RDP (Reality Display Processor) emulation.                                 *
*                                                                              *
\******************************************************************************/




static void rdp_noop()
{
#ifdef LOG_ON
            LOG_TO_FILE("%08X: %08X %08X CMD RDP_NOOP\n",
                 ADDR, CMD0, CMD1);
#endif
} /* static void rdp_noop() */



static void rdp_texrect()
{
        _u32   a = rdp_reg.pc[rdp_reg.pc_i];
        _u32   cmd2 = ((_u32 *)pRDRAM)[(a>>2)+1];
        _u32   cmd3 = ((_u32 *)pRDRAM)[(a>>2)+3];

        _s16  lrx     = (_u16)((rdp_reg.cmd0 & 0x00fff000) >> 12);
        _s16  lry     = (_u16)((rdp_reg.cmd0 & 0x00000fff) >>  0);
        _s16  ulx     = (_u16)((rdp_reg.cmd1 & 0x00fff000) >> 12); 
        _s16  uly     = (_u16)((rdp_reg.cmd1 & 0x00000fff) >>  0);

        float  lrxf     = lrx / 4.0f;
        float  lryf     = lry / 4.0f;
        float  ulxf     = ulx / 4.0f;
        float  ulyf     = uly / 4.0f;

        int     tile    = (_u16)((rdp_reg.cmd1 & 0x07000000) >> 24);

        float   s       = ((float)(_s16)((rdp_reg.cmd2 >> 16) & 0xffff)) / 32.0f;   //  s10.5
        float   t       = ((float)(_s16)((rdp_reg.cmd2      ) & 0xffff)) / 32.0f;   //  s10.5

        float   dsdx    = ((float)((_s16)((rdp_reg.cmd3 >> 16) & 0xffff))) / 1024.0f; //  s5.10
        float   dtdy    = ((float)((_s16)((rdp_reg.cmd3      ) & 0xffff))) / 1024.0f; //  s5.10

//      sprintf(output,"cmd0 = %8x\ncmd1 = %8x\ncmd2 = %8x\ncmd3 = %8x",rdp_reg.cmd0,rdp_reg.cmd1,rdp_reg.cmd2,rdp_reg.cmd3);
//      MessageBox(hGraphics, output, "textrect", MB_OK);

#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD RDP_TEXRECT",
            ADDR, CMD0, CMD1);
        LOG_TO_FILE("\tulx=%+9.4f; uly=%+9.4f; lrx=%+9.4f; lry=%+9.4f;\n\ttile=%u; s=%+9.4f; t=%+9.4f; dsdx=%+7.4f; dtdy=%+7.4f\n",
            ulxf, ulyf, lrxf, lryf, tile, s, t, dsdx, dtdy);
#endif

        rdp_reg.curr = rdp_reg.pc[rdp_reg.pc_i] = (a + 16) & 0x007fffff;   

//      rdp_reg.loadtile  = tile;
//      rdp_reg.m_CurTile = &rdp_reg.td[tile];
        //rdp_reg.m_NexTile = &rdp_reg.td[(tile+1)&7];
//  return;
    /* gil: in this case i am correct! */
    /* 1cyc mode (0x00000000): w = lrx - ulx     */
    /* 2cyc mode (0x00100000): w = lrx - ulx     */
    /* copy mode (0x00200000): w = lrx - ulx + 1 */
    /* fill mode (0x00300000): not allowed       */
        if(cycle_mode == CYCLE_COPY)
        {
            /* we are in copy mode */
            /* 4 texels are drawn per cycle */
//                lry--;
                dsdx /= 4.0;
        }
        else
        {
            /* we are in 1 or 2 cycle mode (fill mode is not allowed)! */
            /* we must not draw the bottom and right edges */
                lrx--;
                lry--;
                lrxf-=1.0f;
                lryf-=1.0f;
        }

        if (CamSpyON)
            Render_TexRectangleFastFB(ulxf, ulyf, lrxf, lryf, tile, s,t,dsdx, dtdy);
        else
            Render_TexRectangle(ulxf, ulyf, lrxf, lryf, tile, s,t,dsdx, dtdy);


} /* static void rdp_texrect() */





static void rdp_texrectflip()
{
#ifdef LOG_ON
            LOG_TO_FILE("%08X: %08X %08X CMD RDP_TEXRECTFLIP",
                 ADDR, CMD0, CMD1);

        LOG_TO_FILE("\tIgnored!\n");
#endif
} /* static void rdp_texrectflip() */





static void rdp_loadsync()
{
#ifdef LOG_ON
            LOG_TO_FILE("%08X: %08X %08X CMD RDP_LOADSYNC",
                 ADDR, CMD0, CMD1);

        LOG_TO_FILE("\tIgnored!\n");
#endif
} /* static void rdp_loadsync() */





static void rdp_pipesync()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_PIPESYNC",
                 ADDR, CMD0, CMD1);
        LOG_TO_FILE("\tIgnored!\n");
#endif
} /* static void rdp_pipesync() */





static void rdp_tilesync()
{
    int i = 01;
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD RDP_TILESYNC",
                 ADDR, CMD0, CMD1);
        LOG_TO_FILE("\tIgnored!\n");
#endif
} /* static void rdp_tilesync() */





static void rdp_fullsync()
{
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD RDP_FULLSYNC\n",
                 ADDR, CMD0, CMD1);
#endif
        *pInterruptMask = 0x20; //0x20 Lionel

} /* static void rdp_fullsync() */





static void rdp_setkeygb()
{
    //_g->words.w0 = (_SHIFTL(G_SETKEYGB, 24, 8) |          \
    //  _SHIFTL(wG, 12, 12) | _SHIFTL(wB, 0, 12));  \
    //_g->words.w1 = (_SHIFTL(cG, 24, 8) | _SHIFTL(sG, 16, 8) | \
    //   _SHIFTL(cB, 8, 8) | _SHIFTL(sB, 0, 8));    
    float Green = (float)((CMD1 >> 24) & 0xff) / 256.0f;
    float Blue = (float)((CMD1>> 8) & 0xff) / 256.0f;

    rdp_reg.key_b = Blue;
    rdp_reg.key_1mb = 1.0f - Blue;

    rdp_reg.key_g = Green;
    rdp_reg.key_1mg = 1.0f - Green;

#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETKEYG",
                 ADDR, CMD0, CMD1);

        LOG_TO_FILE("\tIgnored!\n");
#endif
} /* static void rdp_setkeygb() */





static void rdp_setkeyr()
{
    //_SHIFTL(G_SETKEYR, 24, 8),                    \
    //_SHIFTL(wR, 16, 12) | _SHIFTL(cR, 8, 8) | _SHIFTL(sR, 0, 8)   \

    float Red = (float)((CMD0 >> 8) & 0xff) / 256.0f;
    rdp_reg.key_r = Red;
    rdp_reg.key_1mr = 1.0f - Red;
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETKEYR",
                 ADDR, CMD0, CMD1);
        LOG_TO_FILE("\tIgnored!\n");
#endif
} /* static void rdp_setkeyr() */





static void rdp_setconvert()
{
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETCONVERT",
                 ADDR, CMD0, CMD1);
        LOG_TO_FILE("\tIgnored!\n");
#endif
} /* static void rdp_setconvert() */





static void rdp_setscissor()
{
        static char *interlace_modes[] = { "no", "?", "even", "odd" };

        rdp_reg.scissor.ulx             = (rdp_reg.cmd0 & 0x00fff000) >> 14;
        rdp_reg.scissor.uly             = (rdp_reg.cmd0 & 0x00000fff) >> 2;
        rdp_reg.scissor.interlace_mode  = (rdp_reg.cmd1 & 0x03000000) >> 24;
        rdp_reg.scissor.lrx             = (rdp_reg.cmd1 & 0x00fff000) >> 14;
        rdp_reg.scissor.lry             = (rdp_reg.cmd1 & 0x00000fff) >> 2;

    /* It seems that most demo coders thought that SETSCISSOR uses:
          x, y, width, height
       arguments. That is NOT correct. SETSCISSOR uses:
          ulx, uly, lrx, lry
       args!!! */

#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETSCISSOR",
                 ADDR, CMD0, CMD1);

        LOG_TO_FILE("\tulx=%lu; uly=%lu; lrx=%lu; lry=%lu; interlace_mode=%s\n",
                rdp_reg.scissor.ulx,
                rdp_reg.scissor.uly,
                rdp_reg.scissor.lrx,
                rdp_reg.scissor.lry,
                interlace_modes[rdp_reg.scissor.interlace_mode]
                );
//        LOG_TO_FILE("interlace_mode is ignored\n");
#endif
        SetVisualClipRectangle(rdp_reg.scissor.ulx, rdp_reg.scissor.uly, rdp_reg.scissor.lrx, rdp_reg.scissor.lry);

} /* static void rdp_setscissor() */





static void rdp_setprimdepth()
{
    rdp_reg.primdepth = (CMD1 >> 16) & 0x7FFF;
    rdp_reg.fprimdepth = (float)((CMD1 >> 16) & 0x7FFF) / (float)0x8000;
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETPRIMDEPTH",
                 ADDR, CMD0, CMD1);
        LOG_TO_FILE("\tPrimdeth = %2.6f!\n",rdp_reg.fprimdepth);
#endif
} /* static void rdp_setprimdepth() */





static void rdp_setothermode()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETOTHERMODE\n",
                    ADDR, CMD0, CMD1);
#endif
    rdp_reg.mode_h = rdp_reg.cmd0 & 0x00ffffff;
    rdp_reg.mode_l = rdp_reg.cmd1 & 0xffffffff;

    cycle_mode = (_u8)((rdp_reg.mode_h>>0x14) & 0x3);//^3;

    if ((rdp_reg.mode_l & 0x0c00) != 0xc00) glDepthRange(-1,1.);
    else  glDepthRange(-1,0.9995);

    if(rdp_reg.mode_l & 0x00000010)
        Render_geometry_zbuffer(1);
    else
        Render_geometry_zbuffer(0);

    if (rdp_reg.mode_l & 0x00000020)
        Render_geometry_zwrite(1);
    else
        Render_geometry_zwrite(0);
/*
//  if ((rdp_reg.mode_l & 0x0f0f0000) != 0x50000)
    if ((rdp_reg.mode_l & 0x0f000000) != 0x0)
    {
        Src_Alpha = GL_SRC_ALPHA;
        Dst_Alpha = GL_ONE_MINUS_SRC_ALPHA;
    }
    else
    {
//      Src_Alpha = GL_ONE;
//      Dst_Alpha = GL_ZERO;
    }
*/
} /* static void rdp_setothermode() */

_u32 lTmem;
_u16 lPallete[256];
_u16 *lPallete16[16] = {&lPallete[0x00],&lPallete[0x10],&lPallete[0x20],&lPallete[0x30],
                        &lPallete[0x40],&lPallete[0x50],&lPallete[0x60],&lPallete[0x70],
                        &lPallete[0x80],&lPallete[0x90],&lPallete[0xa0],&lPallete[0xb0],
                        &lPallete[0xc0],&lPallete[0xd0],&lPallete[0xe0],&lPallete[0xf0]};
BOOL loadPalAddr = TRUE;

void UpdatePalette()
{
    int i,pal;
    _u16  *spal = (_u16 *)((_u8 *)pRDRAM + (rdp_reg.tlut_8_addr));
    _u32  color;
    _u8 intensity;
    _u8   *dia = (_u8 *)(&Palette8IA[0]);
    _u8   *drgba = (_u8 *)(&Palette8RGBA[0]);

    for (i=0; i<17; i++)
    {
        PaletteCRC[i] = 0;
    }

    for (i=0; i<256; i++)
    {
        pal = i >> 4;
        PaletteCRC[16] = PaletteCRC[16] << 11 | PaletteCRC[16] >> 21;
        PaletteCRC[16] += ((_u32)lPallete[i])<<pal;// * (i + 1));
        PaletteCRC[16] = PaletteCRC[16] << (15 + (i & 15)) | PaletteCRC[16] >> (17 - (i & 15));
        PaletteCRC[16] ^= ((_u32)lPallete[i])<<(16-pal);// * (i + 1));

        PaletteCRC[pal] = (PaletteCRC[pal] << (5+(i & 15))) | (PaletteCRC[pal] >> (27-(i & 15)));
        PaletteCRC[pal] += ((_u32)(lPallete[i]))<<pal;// * ((i & 0xf) + 1));
        PaletteCRC[pal] = (PaletteCRC[pal] << (21-(i & 15))) | (PaletteCRC[pal] >> (11+(i & 15)));
        PaletteCRC[pal] ^= ((_u32)lPallete[i])<<(16-pal);// * ((i & 0xf) + 1));
    }

    for (i=0; i<17; i++)
    {
        InvPaletteCRC[i] = ~PaletteCRC[i];
    }

    spal = lPallete;

    for (i=0; i<256; i++)
    {
        color = lPallete[i];//*spal++;
        intensity = (_u8)((color >> 8 ) & 0x00ff);
        *drgba++ = (_u8)(((color >> 11 ) & 0x001f)<<3);
        *drgba++ = (_u8)(((color >>  6 ) & 0x001f)<<3);
        *drgba++ = (_u8)(((color >>  1 ) & 0x001f)<<3);
        *drgba++ = (color & 0x01) ? 0xff : 0x00;

        *dia++ = intensity;
        *dia++ = intensity;
        *dia++ = intensity;
        *dia++ = (_u8)(color & 0xff);
    }
}

static void rdp_loadtlut()
{
        _u32  tile   = (rdp_reg.cmd1 >> 24) & 0x07;
        _u16 count  = ((_u16)(rdp_reg.cmd1 >> 14) & 0x03ff)+1;
        _u16 pltofst  = ((_u16)(rdp_reg.cmd1 >> 14) & 0x03);
        _u16 pltstart  = ((_u16)(rdp_reg.cmd1 >> 2) & 0x03ff)+1;
        t_tile *tmpTile = &rdp_reg.td[tile];

        _u16 x0 = (_u16)(((rdp_reg.cmd0 >> 14)) & 0x03ff);
        _u16 y0 = (_u16)(((rdp_reg.cmd0 >> 2 )) & 0x03ff);
        _u16 x1 = (_u16)(((rdp_reg.cmd1 >> 14)) & 0x03ff);
        _u16 y1 = (_u16)(((rdp_reg.cmd1 >> 2 )) & 0x03ff);      

        rdp_reg.tlut_8_fmt  = rdp_reg.TextureImage.fmt;
        rdp_reg.tlut_8_size = rdp_reg.TextureImage.size;
        rdp_reg.tlut_8_addr = (_u32)((rdp_reg.TextureImage.addr));// + (tmpTile->ult * 2));

        lTmem = tmpTile->tmem;

        if (lTmem < 0x100)
            lTmem = 0x100;

        {
            int i;
            int idx;
            int pal = (lTmem  >> 4) & 0xf; //rdp_reg.td[tile].tmem >> 4 & 0xf;
            _u16 *spal;

            if (count  > 256)
                count /= 4;

            if(pltstart!=0)
            {
                spal = (_u16 *)((_u8 *)pRDRAM + (rdp_reg.tlut_8_addr + (2*x0+2*y0)));
                count=(x1-x0)+1;
                if(count>256) count=256;
            }

            if (loadPalAddr)
            {
                for (i=0; i<16; i++) lPallete16[i] = &lPallete[i<<4];
                loadPalAddr=FALSE;
            }

            if ((count + (pal << 4))>256) count = 256 - (pal << 4);


#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_LOADTLUT",
                 ADDR, CMD0, CMD1);
    LOG_TO_FILE("\ttile=%ld (tmem=%lx),(pal=%i)\n\tLoad address = %08x, count=%hx\n", 
               tile,
               rdp_reg.td[tile].tmem,
               (rdp_reg.td[tile].tmem >> 4) & 0x0f,
               rdp_reg.tlut_8_addr,
               count
               );
#endif
            pal = rdp_reg.td[tile].tmem & 0xf0;

            for (idx = 0; idx < count;idx++)
            {
                lPallete[(pal + idx) & 0x0ff] = *spal++;
            }

            UpdatePalette();
        }

        rdp_reg.tlut_4_fmt[rdp_reg.td[tile].palette]  = rdp_reg.TextureImage.fmt;
        rdp_reg.tlut_4_size[rdp_reg.td[tile].palette] = rdp_reg.TextureImage.size;
        rdp_reg.tlut_4_addr[rdp_reg.td[tile].palette] = (_u32)rdp_reg.TextureImage.addr;
} /* static void rdp_loadtlut() */

static void rdp_settilesize()
{
        _u32 tile = (rdp_reg.cmd1 >> 24) & 0x07;
        _u32 oldtile = rdp_reg.tile;
        t_tile *savCurTile = rdp_reg.m_CurTile;
        int twidth, theight;
        t_tile *tmpTile = &(rdp_reg.td[tile]);

        if (tile == 7)
            tile += 0;

        rdp_reg.m_CurTile = tmpTile;
        rdp_reg.tile = tile;

        tmpTile->uls = ((_u16)(rdp_reg.cmd0 >> 12)) & 0x0fff;
        tmpTile->ult = ((_u16)( rdp_reg.cmd0     )) & 0x0fff;
        tmpTile->lrs = ((_u16)(rdp_reg.cmd1 >> 12)) & 0x0fff;
        tmpTile->lrt = ((_u16)( rdp_reg.cmd1     )) & 0x0fff;

        tmpTile->uls = (tmpTile->uls >> 2);
        tmpTile->ult = (tmpTile->ult >> 2);

        tmpTile->lrs = ((tmpTile->lrs) >> 2);
        tmpTile->lrt = ((tmpTile->lrt) >> 2);

        twidth  = tmpTile->lrs - tmpTile->uls + 1;
        tmpTile->Width  = twidth;

        if ((tmpTile->masks) && (tmpTile->masks < MAXSMASK))
        {
            tmpTile->Width = 1<<tmpTile->masks;
//          if ((twidth > tmpTile->Width) || tmpTile->mirrors) tmpTile->clamps = 0;
            if (twidth > (int)tmpTile->Width) tmpTile->clamps = 0;
        }

        theight  = tmpTile->lrt - tmpTile->ult + 1;

        tmpTile->Height  = theight;

        if ((tmpTile->maskt) && ((tmpTile->maskt < MAXTMASK)))
        {
            tmpTile->Height = 1<<tmpTile->maskt;
//          if ((theight > tmpTile->Height) || tmpTile->mirrort) tmpTile->clampt = 0;
            if (theight > (int)tmpTile->Height) tmpTile->clampt = 0;
        }

        if (tmpTile->maskt == 0) tmpTile->clampt = 1;
        if (tmpTile->clampt) tmpTile->mirrort = 0;
//      if (tmpTile->maskt == 0) tmpTile->mirrort = 0;

        if (tmpTile->masks == 0) tmpTile->clamps = 1;
        if (tmpTile->clamps) tmpTile->mirrors = 0;
//      if (tmpTile->masks == 0) tmpTile->mirrors = 0;

//**    Math the Texture-Coordinate Scalefactors ...
        MathTextureScales();

        rdp_reg.tile = oldtile;
        rdp_reg.m_CurTile = savCurTile;

#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETTILESIZE",
            ADDR, CMD0, CMD1);

        LOG_TO_FILE("\tuls=%hu; ult=%hu; lrs=%hu; lrt=%hu; tile=%lu\n",
            rdp_reg.td[tile].uls, rdp_reg.td[tile].ult,
            rdp_reg.td[tile].lrs, rdp_reg.td[tile].lrt,
            tile);
#endif
} /* static void rdp_settilesize() */


static void rdp_loadblock()
{
        _u32 offset = 0;
        _u32 size = (1 << rdp_reg.TextureImage.size)/2;
        _u32 width = rdp_reg.TextureImage.width;
        _u32 Length = ((_u32)((rdp_reg.cmd1 >> 12) & 0xFFF) + 1);
        _u16 LineWidth;
        
        _u32 tile = (_u32)((rdp_reg.cmd1 >> 24) & 0x07);
        
        _u16 dxt = (_u16)(rdp_reg.cmd1 & 0x0fff);
        t_tile *tmpTile = &(rdp_reg.td[tile]);
//      int i;
/*
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0,              \
        (((width)*(height) + siz##_INCR) >> siz##_SHIFT)-1, 0 ),\

#define G_IM_SIZ_4b_SHIFT  2
#define G_IM_SIZ_8b_SHIFT  1
#define G_IM_SIZ_16b_SHIFT 0
#define G_IM_SIZ_32b_SHIFT 0

#define G_IM_SIZ_4b_INCR  3
#define G_IM_SIZ_8b_INCR  1
#define G_IM_SIZ_16b_INCR 0
#define G_IM_SIZ_32b_INCR 0
*/
        switch(size)
        {
        case 0:
        case 1:
        case 2:
            break;
        case 4:
            Length <<= 1; // ????? not sure about this
        }

        Length >>= 2; // convert to 64bit words

        Length++;

        if (size == 0)
            size = 1;

        tmpTile->reload = TRUE;
        tmpTile->set_by = RDPTD_LOADBLOCK;
        tmpTile->addr   = (_u32)&pRDRAM[rdp_reg.TextureImage.addr];

        if (dxt)
            dxt = (_u16)((2048.0f / (float)(dxt))+0.5f);
        else
            dxt = 0;

        tmpTile->dxt = dxt;

        if (tmpTile->uls != 0)
            size += 0;

        tmpTile->offset = offset;
        
        if (width<=1)
            LineWidth=dxt * 8;
        else
        {
            LineWidth = (_u16)width;
            switch (rdp_reg.TextureImage.size)
            {
            case 0:
                break;
            case 1:
                break;
            case 2:
                LineWidth <<= 1;
                break;
            case 3:
                LineWidth <<= 2;
                break;
            }
        }

    offset = 0;
            if(tmpTile->tmem == 0x160)
                tmpTile->tmem = 0x160;
    {
        LstLdAddr[LstLdAddNum].Addr = rdp_reg.TextureImage.addr + offset;
        LstLdAddr[LstLdAddNum].Offset = tmpTile->tmem;
        LstLdAddr[LstLdAddNum].End = tmpTile->tmem + Length;

        LstLdAddr[LstLdAddNum].LoadedBy = 0;

        LstLdAddr[LstLdAddNum].Pitch = 0;
        LstLdAddr[LstLdAddNum].LineWidth = LineWidth;

        if ((_u16)(rdp_reg.cmd1 & 0x0fff) == 0)
            LstLdAddr[LstLdAddNum].Swapped=1;// | ((tmpTile->ult & 1) << 1);
        else
            LstLdAddr[LstLdAddNum].Swapped=0;

        LstLdAddr[LstLdAddNum].StepAdj=0;

        LstLdAddNum++;
        LstLdAddNum &= LstLdAddrMask;
    }

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_LOADBLOCK",
                 ADDR, CMD0, CMD1);

    LOG_TO_FILE("\tuls=%x; ult=%x; lrs=%x; dxt=%x;\n\ttile=%x (->tmem=0x%x); loadaddr = 0x%08x; length=0x%x\n",
        rdp_reg.td[tile].uls, rdp_reg.td[tile].ult,
        rdp_reg.td[tile].lrs, rdp_reg.td[tile].dxt,
        tile,rdp_reg.td[tile].tmem,(rdp_reg.TextureImage.addr + offset),Length);
#endif
    lTmem = tmpTile->tmem;

    if (lTmem < 0x100) return;

    {
        _u16 count  = ((_u16)(rdp_reg.cmd1 >> 14) & 0x03ff);
        int i;
        int idx;
        int pal = (lTmem  >> 4) & 0xf; //rdp_reg.td[tile].tmem >> 4 & 0xf;
        _u16  *spal = (_u16 *)((_u8 *)pRDRAM + (rdp_reg.tlut_8_addr));
//      _u32  color;
//      _u8 intensity;
        _u8   *dia = (_u8 *)(&Palette8IA[0]);
        _u8   *drgba = (_u8 *)(&Palette8RGBA[0]);
        _u16 pltstart  = ((_u16)(rdp_reg.cmd1 >> 2) & 0x03ff)+1;

        _u16 uls = (_u16)(((rdp_reg.cmd0 >> 14)) & 0x03ff);
        _u16 ult = (_u16)(((rdp_reg.cmd0 >> 2 )) & 0x03ff);
        _u16 lrs = (_u16)(((rdp_reg.cmd1 >> 14)) & 0x03ff);
        _u16 lrt = (_u16)(((rdp_reg.cmd1 >> 2 )) & 0x03ff);     

        rdp_reg.tlut_8_addr = (_u32)(rdp_reg.TextureImage.addr);

        if (loadPalAddr)
        {
            for (i=0; i<16; i++)
                lPallete16[i] = &lPallete[i<<4];
            loadPalAddr=FALSE;
        }

        if(pltstart!=0)
        {
            spal = (_u16 *)((_u8 *)pRDRAM + (rdp_reg.tlut_8_addr + (2*uls+2*ult)));
            count=(lrs-uls)+1;
            if(count>256) count=256;
        }

        pal <<= 4;

        if ((count + pal) > 256) count = 256 - pal;

        for (idx = 0; idx < count;idx++)
        {
            lPallete[(pal+idx) & 0x0ff] = *spal++;
        }

        UpdatePalette();
    }

} /* static void rdp_loadblock() */


static void rdp_loadtile()
{
        _u32 offset;
        _u32 size = (1 << rdp_reg.TextureImage.size)/2;
        _u32 width = rdp_reg.TextureImage.width;
        _u16 LineWidth;
        _u32 tile = (_u32)((rdp_reg.cmd1 >> 24) & 0x07);
        t_tile *tmpTile = &(rdp_reg.td[tile]);

        _u16 uls = (_u16)(((rdp_reg.cmd0 >> 12)) & 0x0fff);
        _u16 ult = (_u16)(((rdp_reg.cmd0      )) & 0x0fff);
        _u16 lrs = (_u16)(((rdp_reg.cmd1 >> 12)) & 0x0fff);
        _u16 lrt = (_u16)(((rdp_reg.cmd1      )) & 0x0fff);     

        _u32 Width = lrs - uls + 4;
        _u32 Height = lrt - ult + 4;

        _u32 Length;

        uls = (uls >> 2);
        ult = (ult >> 2);
        lrs = ((lrs) >> 2);
        lrt = ((lrt) >> 2);

        Width >>= 2;
        Height >>= 2;

        Length = Width * Height;

//      if (width < 0) width = - width;
//      if (size == 0)
//          size = 1;

        tmpTile->reload = TRUE;
        tmpTile->set_by = RDPTD_LOADTILE;
        tmpTile->addr   = (_u32)&pRDRAM[rdp_reg.TextureImage.addr];//(_u32)rdp_reg.TextureImage.addr;

        rdp_reg.loadtile  = tile;

        if (width<=1)
            LineWidth=(_u16)Width;
        else
            LineWidth=(_u16)width;

        switch (rdp_reg.TextureImage.size)
        {
        case 0:
            Length >>= 4;
            size = 1;
            break;
        case 1:
            Length >>= 3;
            size = 2;
            break;
        case 2:
            Length >>= 2;
            LineWidth <<= 1;
            size = 4;
            break;
        case 3:
            Length >>= 1;
            LineWidth <<= 2;
            size = 8;
            break;
        }

        if (width <= 1)
        {
            if (size > 1)
                width = Width;// >> 1;
            else
                width = Width;// >> 1;
        }

        offset = (uls + (ult * width)) * size;
        offset >>= 1;

        tmpTile->addr   = rdp_reg.TextureImage.addr;
        tmpTile->offset = offset;

        offset += rdp_reg.TextureImage.addr;

        Length++;
        
        LstLdAddr[LstLdAddNum].Addr=offset;
        LstLdAddr[LstLdAddNum].Offset=tmpTile->tmem;
        LstLdAddr[LstLdAddNum].End = tmpTile->tmem + Length;
        LstLdAddr[LstLdAddNum].Pitch=width;
        LstLdAddr[LstLdAddNum].LineWidth=LineWidth;
        LstLdAddr[LstLdAddNum].Swapped=0;
        LstLdAddr[LstLdAddNum].StepAdj=(_u16)(tmpTile->uls * size) &0x03;

        LstLdAddr[LstLdAddNum].LoadedBy = 1;

        LstLdAddNum++;
        LstLdAddNum&=LstLdAddrMask;
    

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_LOADTILE",
        ADDR, CMD0, CMD1);

    LOG_TO_FILE("\tuls=%lu, ult=%lu, lrs=%lu, lrt=%lu, tile=%lu\n\tlength = 0x%x", 
        tmpTile->uls, tmpTile->ult,
        tmpTile->lrs, tmpTile->lrt, 
        tile, Length);
#endif
} /* static void rdp_loadtile() */





static void rdp_settile()
{
        _u32 oldtile = rdp_reg.tile;
        _u32 tile = (_u32)((rdp_reg.cmd1 >> 24) & 0x07);
        t_tile *tmpTile = &(rdp_reg.td[tile]);
    
        t_tile *savCurTile = rdp_reg.m_CurTile;

        tmpTile->format  = (_u8) ((rdp_reg.cmd0 >> 21) & 0x07);
        tmpTile->size    = (_u8) ((rdp_reg.cmd0 >> 19) & 0x03);
        tmpTile->line    = (_u16)((rdp_reg.cmd0 >>  9) & 0x01ff) * 8;
        tmpTile->tmem    = (_u16)( rdp_reg.cmd0        & 0x01ff);

        tmpTile->palette = (_u8) ((rdp_reg.cmd1 >> 20) & 0x0f);

        tmpTile->clampt  = (_u8) ((rdp_reg.cmd1 >> 19) & 0x01);
        tmpTile->mirrort = (_u8) ((rdp_reg.cmd1 >> 18) & 0x01);
        tmpTile->maskt   = (_u8) ((rdp_reg.cmd1 >> 14) & 0x0f);
        tmpTile->shiftt  = (_u8) ((rdp_reg.cmd1 >> 10) & 0x0f);

        tmpTile->clamps  = (_u8) ((rdp_reg.cmd1 >>  9) & 0x01);
        tmpTile->mirrors = (_u8) ((rdp_reg.cmd1 >>  8) & 0x01);
        tmpTile->masks   = (_u8) ((rdp_reg.cmd1 >>  4) & 0x0f);
        tmpTile->shifts  = (_u8) ( rdp_reg.cmd1        & 0x0f);

        tmpTile->addr    = (_u32)&pRDRAM[rdp_reg.TextureImage.addr];

        lTmem = tmpTile->tmem;
            if(tmpTile->tmem == 0x160)
                tmpTile->tmem = 0x160;

        pltmode = (rdp_reg.mode_h & 0x0000c000);

        //** setup line from "64bit pixel" to correct 32, 16, 8 or 4bit pixel 
//        switch(rdp_reg.td[tile].size)
//        switch(rdp_reg.TextureImage.size)

        switch(rdp_reg.td[tile].size)
        {
                case 0:   //** 4bit 
//                        rdp_reg.td[tile].line <<= 4;
                        break;
                case 1:   //** 8bit 
//                        rdp_reg.td[tile].line <<= 3;
                        break;
                case 2:   //** 16bit 
//                        rdp_reg.td[tile].line <<= 2;
                        break;
                case 3:   //** 32bit 
//                        rdp_reg.td[tile].line <<= 2;//1;
                        rdp_reg.td[tile].line <<= 1;
                        break;
        }

        if (tile == 7)
            tile += 0;
    rdp_reg.tile = tile;
    rdp_reg.m_CurTile = tmpTile;

/*
    if(tile!=7)
    {
        DWORD last=LstLdAddNum;
        if(last) last--;
        else last=7;
        if(LstLdAddr[last].Offset != tmpTile->tmem)
        {           
            tmpTile->reload = TRUE;
            LstLdAddr[LstLdAddNum].Pitch=0;
            LstLdAddr[LstLdAddNum].LineWidth=LstLdAddr[last].LineWidth;
            LstLdAddr[LstLdAddNum].Swapped=LstLdAddr[last].Swapped;
            LstLdAddr[LstLdAddNum].Addr=LstLdAddr[last].Addr;
            LstLdAddr[LstLdAddNum].Offset=tmpTile->tmem;
            LstLdAddNum++;
            LstLdAddNum&=7;
        }
    }
        static char *format[]   = { "RGBA", "YUV", "CI", "IA", "I", "?", "?", "?" };
        static char *size[]     = { "4bit", "8bit", "16bit", "32bit" };
        static char *cm[]       = { "NOMIRROR/WARP(NOCLAMP)", "MIRROR", "CLAMP", "MIRROR&CLAMP" };

*/
    MathTextureScales();

    rdp_reg.m_CurTile = savCurTile;
    rdp_reg.tile = oldtile;

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETTILE",
        ADDR, CMD0, CMD1);
    LOG_TO_FILE("\ttile = %i, format = %s, size = %s, line = %i, tmem = 0x%x, palette = %i",
        tile,format[tmpTile->format], size[tmpTile->size],
        tmpTile->line, tmpTile->tmem, tmpTile->palette);
    LOG_TO_FILE("\tclamps = %3i, mirrors = %3i, masks = %3i, shifs = %3i",
        tmpTile->clamps, tmpTile->mirrors, tmpTile->masks, tmpTile->shifts);
    LOG_TO_FILE("\tclampt = %3i, mirrort = %3i, maskt = %3i, shift = %3i\n",
        tmpTile->clampt, tmpTile->mirrort, tmpTile->maskt, tmpTile->shiftt);
#endif
} /* static void rdp_settile() */


static void rdp_fillrect()
{
        _u32   ulx, uly, lrx, lry;
        _u32   color;

        lrx = (rdp_reg.cmd0 & 0x00fff000) >> 12;
        lry = (rdp_reg.cmd0 & 0x00000fff) >> 0;
        ulx = (rdp_reg.cmd1 & 0x00fff000) >> 12;
        uly = (rdp_reg.cmd1 & 0x00000fff) >> 0;

#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD RDP_FILLRECT",
                 ADDR, CMD0, CMD1);
        
        LOG_TO_FILE("\tulx=%lu; uly=%lu; lrx=%lu; lry=%lu",
                           ulx/4,
                           uly/4,
                           lrx/4,
                           lry/4);
        LOG_TO_FILE("\tmodes are ignored\n");
#endif
        /* modes: copy mode is not used (not allowed?) */
        if(!(rdp_reg.mode_h & 0x00200000))
        {
            /* if we are not in 1 or 2 cycle mode (not in copy or fill mode) */
            /* we must not draw the bottom and right edges                   */
                //lrx--;
                //lry--;
                color = rdp_reg.blendcolor;
        }
        else
        {
                color = rdp_reg.fillcolor;
        }
                SetVisualColor(rdp_reg.fillcolor);

                Render_FillRectangle(ulx, uly, lrx, lry, color);
} /* static void rdp_fillrect() */





static void rdp_setfillcolor()
{
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETFILLCOLOR",
                 ADDR, CMD0, CMD1);

        LOG_TO_FILE("\tcolor=$%08lx\n", rdp_reg.cmd1);
#endif
        rdp_reg.fillcolor = rdp_reg.cmd1;

        rdp_reg.fill_r = ((float)((rdp_reg.fillcolor >> 11) & 0x1f) / 31.0f);
        rdp_reg.fill_g = ((float)((rdp_reg.fillcolor >>  6) & 0x1f) / 31.0f);
        rdp_reg.fill_b = ((float)((rdp_reg.fillcolor >>  1) & 0x1f) / 31.0f);
        rdp_reg.fill_a = (float)(rdp_reg.fillcolor & 0x1);

        rdp_reg.fill_1mr = 1.0f - rdp_reg.fill_r;
        rdp_reg.fill_1mg = 1.0f - rdp_reg.fill_g;
        rdp_reg.fill_1mb = 1.0f - rdp_reg.fill_b;
        rdp_reg.fill_1ma = 1.0f - rdp_reg.fill_a;

} /* static void rdp_setfillcolor() */





static void rdp_setfogcolor()
{
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETFOGCOLOR",
                 ADDR, CMD0, CMD1);

        LOG_TO_FILE("\tcolor=$%08lx\n", rdp_reg.cmd1);
#endif
        rdp_reg.fogcolor = rdp_reg.cmd1;

        rdp_reg.fog_r = (((rdp_reg.fogcolor >> 24) & 0xff) / 255.0f);
        rdp_reg.fog_g = (((rdp_reg.fogcolor >> 16) & 0xff) / 255.0f);
        rdp_reg.fog_b = (((rdp_reg.fogcolor >>  8) & 0xff) / 255.0f);
        rdp_reg.fog_a = (rdp_reg.fogcolor & 0xff) / 255.0f;

        rdp_reg.fog_1mr = 1.0f - rdp_reg.fog_r;
        rdp_reg.fog_1mg = 1.0f - rdp_reg.fog_g;
        rdp_reg.fog_1mb = 1.0f - rdp_reg.fog_b;
        rdp_reg.fog_1ma = 1.0f - rdp_reg.fog_a;

} /* static void rdp_setfogcolor() */





static void rdp_setblendcolor()
{
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETBLENDCOLOR",
                 ADDR, CMD0, CMD1);
        LOG_TO_FILE("\tcolor=$%08lx\n", rdp_reg.cmd1);
#endif
        rdp_reg.blendcolor = rdp_reg.cmd1;

        rdp_reg.blend_r = (((rdp_reg.blendcolor >> 24) & 0xff) / 255.0f);
        rdp_reg.blend_g = (((rdp_reg.blendcolor >> 16) & 0xff) / 255.0f);
        rdp_reg.blend_b = (((rdp_reg.blendcolor >>  8) & 0xff) / 255.0f);
        rdp_reg.blend_a = (rdp_reg.blendcolor & 0xff) / 255.0f;

        rdp_reg.blend_1mr = 1.0f - rdp_reg.blend_r;
        rdp_reg.blend_1mg = 1.0f - rdp_reg.blend_g;
        rdp_reg.blend_1mb = 1.0f - rdp_reg.blend_b;
        rdp_reg.blend_1ma = 1.0f - rdp_reg.blend_a;

} /* static void rdp_setblendcolor() */





static void rdp_setprimcolor()
{
    _u8 L = CMD0 & 0xff;
    _u8 M = (CMD0 >> 8) & 0xff;
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETPRIMCOLOR",
                 ADDR, CMD0, CMD1);
        LOG_TO_FILE("\tcolor=$%08lx", CMD1);
        LOG_TO_FILE("\tM = $%02x; L = $%02x\n", M,L);
#endif
        rdp_reg.primcolor = rdp_reg.cmd1;
        
        rdp_reg.prim_r = (((rdp_reg.primcolor >> 24) & 0xff) / 255.0f);
        rdp_reg.prim_g = (((rdp_reg.primcolor >> 16) & 0xff) / 255.0f);
        rdp_reg.prim_b = (((rdp_reg.primcolor >>  8) & 0xff) / 255.0f);
        rdp_reg.prim_a = (rdp_reg.primcolor & 0xff) / 255.0f;

        rdp_reg.prim_1mr = 1.0f - rdp_reg.prim_r;
        rdp_reg.prim_1mg = 1.0f - rdp_reg.prim_g;
        rdp_reg.prim_1mb = 1.0f - rdp_reg.prim_b;
        rdp_reg.prim_1ma = 1.0f - rdp_reg.prim_a;

        //UpdateTexRectClrs1 |= UsePrimAll;
        //UpdateTexRectClrs2 |= UsePrimAll;
        //UpdateTrisClrs1 |= UsePrimAll;
        //UpdateTrisClrs2 |= UsePrimAll;
} /* static void rdp_setprimcolor() */

static void rdp_setenvcolor()
{
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETENVCOLOR",
                 ADDR, CMD0, CMD1);
        LOG_TO_FILE("\tcolor=$%08lx\n", rdp_reg.cmd1);
#endif
        rdp_reg.envcolor = rdp_reg.cmd1;

        rdp_reg.env_r = (((rdp_reg.envcolor >> 24) & 0xff) / 255.0f);
        rdp_reg.env_g = (((rdp_reg.envcolor >> 16) & 0xff) / 255.0f);
        rdp_reg.env_b = (((rdp_reg.envcolor >>  8) & 0xff) / 255.0f);
        rdp_reg.env_a = (rdp_reg.envcolor & 0xff) / 255.0f;

        rdp_reg.env_1mr = 1.0f - rdp_reg.env_r;
        rdp_reg.env_1mg = 1.0f - rdp_reg.env_g;
        rdp_reg.env_1mb = 1.0f - rdp_reg.env_b;
        rdp_reg.env_1ma = 1.0f - rdp_reg.env_a;

        //UpdateTexRectClrs1 |= UseEnvAll;
        //UpdateTexRectClrs2 |= UseEnvAll;
        //UpdateTrisClrs1 |= UseEnvAll;
        //UpdateTrisClrs2 |= UseEnvAll;

} /* static void rdp_setenvcolor() */

static void rdp_settextureimage()
{
        static char *format[]   = { "RGBA", "YUV", "CI", "IA", "I", "?", "?", "?" };
        static char *size[]     = { "4bit", "8bit", "16bit", "32bit" };


        rdp_reg.TextureImage.fmt      = (_u8)((rdp_reg.cmd0 >> 21) & 0x07);
        rdp_reg.TextureImage.size     = (_u8)((rdp_reg.cmd0 >> 19) & 0x03);
        rdp_reg.TextureImage.width    = (_u16)(1 + (rdp_reg.cmd0 & 0x00000fff));
        rdp_reg.TextureImage.addr     = segoffset2addr(rdp_reg.cmd1);
        rdp_reg.TextureImage.vaddr    = rdp_reg.cmd0;

//              if (ADDR == 0x0041C810) rdp_reg.pc[rdp_reg.pc_i] = 0x0041F7F8;
        CamSpyON = FALSE;
        if (ucode_version == 5) 
        {
            _s32 offset = rdp_reg.TextureImage.addr - rdp_reg.colorimg_addr;
//          _u32   a,ta;
            int cnt = 0;

//          a = (rdp_reg.pc[rdp_reg.pc_i]+0x40) & 0x007fffff;
//          ta = a >> 2;
            CamSpyON = FALSE;

            if ((offset > 0) && (offset <= 320*240*2))
            {
                if (CamSpyGo)
                    Render_FrameBuffer5(); // copy frame buffer we do this 1 time per dlist
                CamSpyGo = FALSE;
                CamSpyON = TRUE;
/*              while((((_u32 *)pRDRAM)[ta + 0] != 0xFD10013F) && (cnt++ < 5))
                {
                    a += 8;
                    a &= 0x007fffff;
                    ta = a >> 2;
                }
                while((((_u32 *)pRDRAM)[ta + 0] == 0xFD10013F))// || (((_u32 *)pRDRAM)[ta + 0] != 0x00000000))
                {
                    a += 48;
                    a &= 0x007fffff;
                    ta = a >> 2;
                }
                rdp_reg.pc[rdp_reg.pc_i] = a - 8; // skip camspy lens code
//              rdp_reg.pc[rdp_reg.pc_i] += 0x0041F7E0 - 0x0041C810; // skip camspy lens code
*/
            }
        }

//      rdp_reg.TextureImage.LoadAddr = &pRDRAM[rdp_reg.TextureImage.addr];

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETTEXTUREIMAGE",
                 ADDR, CMD0, CMD1);

    LOG_TO_FILE("\tformat=%s; size=%s; width=%hu; addr=$%08lx\n",
               format[rdp_reg.TextureImage.fmt],
               size[rdp_reg.TextureImage.size],
               rdp_reg.TextureImage.width,
               rdp_reg.TextureImage.addr);
#endif
} /* static void rdp_settextureimage() */





static void rdp_setdepthimage()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETDEPTHIMAGE",
                 ADDR, CMD0, CMD1);

        LOG_TO_FILE("\taddr=$%lx\n", rdp_reg.cmd1);
#endif
        rdp_reg.depthimg_addr = segoffset2addr(rdp_reg.cmd1);

} /* static void rdp_setdepthimage() */


_u32 LastColorAdd = 0;
_u32 NewColorAdd = 0;

static void rdp_setcolorimage()
{
        static char *format[]   = { "RGBA", "YUV", "CI", "IA", "I", "?", "?", "?" };
        static char *size[]     = { "4bit", "8bit", "16bit", "32bit" };

        //rdp_reg.colorimg_addr = segoffset2addr(rdp_reg.cmd1);
        NewColorAdd = segoffset2addr(rdp_reg.cmd1);

        if ((swapMode == 0) && (NewColorAdd == rdp_reg.depthimg_addr))
        {
            swapmodecnt--;
            if (swapmodecnt == 0) swapMode = 1;
//          SwapNow = TRUE;
        }
/**/
        if ((swapMode == 1)
            && (NewColorAdd != rdp_reg.depthimg_addr)
            && (NewColorAdd != LastColorAdd)
//          && (SwapNow)
            )
        {
            rdp_reg.colorimg_addr = LastColorAdd;
//          if (SwapNow)
                Render_FrameBuffer();
            if (SwapNow)
            {
                Render_FlushVisualRenderBuffer();
                Render_ClearVisual();
                SwapNow = FALSE;
                CamSpyGo = TRUE;
            }
            LastColorAdd = NewColorAdd;
        }
/*
        if ((swapMode == 1)
            && (NewColorAdd != rdp_reg.depthimg_addr))
        {
            LastColorAdd = NewColorAdd;
        }
/**/
        rdp_reg.colorimg_fmt   = (_u8)((rdp_reg.cmd0 & 0x00e00000) >> 21);
        rdp_reg.colorimg_size  = (_u8)((rdp_reg.cmd0 & 0x00180000) >> 19);
        rdp_reg.colorimg_width = (_u16)((rdp_reg.cmd0 & 0x00000fff)+1);
        rdp_reg.colorimg_addr = NewColorAdd;
        rdp_reg.colorimg_addr2 = ~NewColorAdd;

        imgWidth = rdp_reg.colorimg_width;
        imgHeight = (imgWidth / 4.0f) * 3.0f;
        //if(vi_Hires)
        //  imgHeight *= 2;


#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETCOLORIMAGE",
                 ADDR, CMD0, CMD1);

    LOG_TO_FILE("\tformat=%s; size=%s; width=%hu; addr=$%lx\n",
        format[rdp_reg.colorimg_fmt],
        size[rdp_reg.colorimg_size],
        rdp_reg.colorimg_width,
        rdp_reg.colorimg_addr);
#endif
} /* static void rdp_setcolorimage() */





static void rdp_trifill()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_TRIFILL",
                 ADDR, CMD0, CMD1);
#endif
} /* static void rdp_trifill() */





static void rdp_trishade()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_TRISHADE",
                 ADDR, CMD0, CMD1);
#endif
} /* static void rdp_trishade() */





static void rdp_tritxtr()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_TRITXTR",
                 ADDR, CMD0, CMD1);
#endif
} /* static void rdp_tritxtr() */





static void rdp_trishadetxtr()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_TRISHADETXTR",
                 ADDR, CMD0, CMD1);
#endif
} /* static void rdp_trishadetxtr() */





static void rdp_trifillz()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_TRIFILLZ",
                 ADDR, CMD0, CMD1);
#endif
} /* static void rdp_trifillz() */





static void rdp_trishadez()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_TRISHADEZ",
                 ADDR, CMD0, CMD1);
#endif
} /* static void rdp_trishadez() */





static void rdp_tritxtrz()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_TRITXTRZ",
                 ADDR, CMD0, CMD1);
#endif
} /* static void rdp_tritxtrz() */





static void rdp_trishadetxtrz()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_TRISHADETXTRZ",
                 ADDR, CMD0, CMD1);
#endif
} /* static void rdp_trishadetxtrz() */




static void rsp_texture()
{
        int idx;
        int tile = (rdp_reg.cmd0 >> 8)  & 0x07;             //** tile t0
        int tile1 = (tile + 1)  & 0x07;                     //** tile t1
        _u32 mipmap_level = (rdp_reg.cmd0 >> 11) & 0x07;    //** mipmap_level   - not used yet
        _u32 on = (rdp_reg.cmd0 & 0xff);            //** 1: on - 0:off

        float s = (float)((rdp_reg.cmd1 >> 16) & 0xffff);
        float t = (float)((rdp_reg.cmd1      ) & 0xffff);
        float SScale;
        float TScale;

        t_tile *tmp_tile = &rdp_reg.td[tile];
        t_tile *tmp_tile1 = &rdp_reg.td[tile1];
        tmp_tile->Texture_on = (_u8)on;

        rdp_reg.tile = tile;

        if (s<=1)
            SScale=1.0f;
        else
            SScale=(float)s/65535.f;

        if (t<=1)
            TScale=1.0f;
        else
            TScale=(float)t/65535.f;

        TScale/=32.f;
        SScale/=32.f;

        for (idx=0;idx<8;idx++)
        {
            rdp_reg.tile = idx;
            tmp_tile1 = &rdp_reg.td[idx];
            tmp_tile1->SScale=SScale;
            tmp_tile1->TScale=TScale;
            rdp_reg.m_CurTile = tmp_tile1;
            MathTextureScales();
        }

        rdp_reg.tile = tile;
        rdp_reg.m_CurTile = tmp_tile;
//      MathTextureScales();

#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC4_TEXTURE",ADDR, CMD0, CMD1);
        LOG_TO_FILE("\tTile = %i, Mipmap = %i, enambled = %s\n",tile, mipmap_level, (on)?"on":"off");
#endif
}




void UpdateTile(t_tile *tmpTile)
{

    BOOL found=FALSE;
    _u16 tries=0;
    _u16 closesttmem = 0;
    _u16 closestidx = ((_u16)LstLdAddNum - 1) & LstLdAddrMask;

    _u16 lp=(_u16)LstLdAddNum;

    tmpTile->addr = (_u32)&pRDRAM[0];
    
    if(lp)
        lp--;
    else
        lp=LstLdAddrMask;

    closestidx = lp;

    tmpTile->Pitch = LstLdAddr[lp].Pitch;
    tmpTile->offset =LstLdAddr[lp].Addr + (_u32)(tmpTile->tmem - LstLdAddr[lp].Offset) * 8;
    tmpTile->LineWidth = (_u16)LstLdAddr[lp].LineWidth;

    if (LstLdAddr[lp].Swapped>0)
        tmpTile->WdSwpd = TRUE;
    else
        tmpTile->WdSwpd = FALSE;
    tries=0;

    while(!found)
    {
        if ((LstLdAddr[lp].Offset <= tmpTile->tmem) 
         && (LstLdAddr[lp].End    >  tmpTile->tmem))
        {
            //if(tmpTile->tmem == 0x160)
            //  tmpTile->tmem = 0x2C;
            tmpTile->Pitch = LstLdAddr[lp].Pitch;
            tmpTile->offset =LstLdAddr[lp].Addr + (_u32)(tmpTile->tmem - LstLdAddr[lp].Offset) * 8;
            tmpTile->LineWidth = (_u16)LstLdAddr[lp].LineWidth;

            if (LstLdAddr[lp].Swapped>0)
                tmpTile->WdSwpd = TRUE;
            else
                tmpTile->WdSwpd = FALSE;
            found=TRUE;
        }
        else
        {
            if(lp)
                lp--;
            else
                lp=LstLdAddrMask;
        }
        tries++;
        if(tries == LstLdAddrSize)
        {
            tries++;
            break;
        }
    }

    if(!found)
    {
        lp = closestidx;
        tmpTile->Pitch = LstLdAddr[lp].Pitch;
        tmpTile->LineWidth = (_u16)LstLdAddr[lp].LineWidth;
        tmpTile->offset = LstLdAddr[lp].Addr + (_u32)(tmpTile->tmem - LstLdAddr[lp].Offset) * 8;

        if (LstLdAddr[lp].Swapped>0)
            tmpTile->WdSwpd = TRUE;
        else
            tmpTile->WdSwpd = FALSE;
        
    }

    switch(tmpTile->size)
    {
    case 0:
        tmpTile->Pitch<<=1;
        break;
    case 2:
        break;
    case 3:
        break;
    }

    if(tmpTile->Pitch<=01)
    {
        tmpTile->Pitch=tmpTile->line;
    }
}

