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

#include "type_sizes.h"
#include "debug.h"

int CheckDList(unsigned __int32 dwAddr, int check_ucode);
int AutodetectUCode(unsigned __int32 dwAddr);

_u32 auto_halt;
_u32 auto_wrong_ucode;
_u32 auto_counter;
_u32 auto_cmd0, auto_cmd1;
_u32 auto_pc[10], auto_pc_i;
_u32 auto_segment[16] = {0xffffffff,0xffffffff,0xffffffff,0xffffffff,
                         0xffffffff,0xffffffff,0xffffffff,0xffffffff,
                         0xffffffff,0xffffffff,0xffffffff,0xffffffff,
                         0xffffffff,0xffffffff,0xffffffff,0xffffffff};

extern int              *pInterruptMask;
extern unsigned char    *pRDRAM;
extern unsigned char    *pIDMEM;
extern unsigned char    *pVIREG;


//////////////////////////////////////////////////////////////////////////////
// standards                                                                //
//////////////////////////////////////////////////////////////////////////////

_u32 segoffset2addr(_u32 so)
{
    _u32 seg = (so>>24)&0x0f;

    if (auto_segment[seg] != 0xffffffff)
        return((auto_segment[seg] + (so&0x00ffffff))  & 0x007fffff );
    else
        return((auto_segment[0] + (so&0x00ffffff))  & 0x007fffff );
} 

void auto_rdp_cmd()
{}

void auto_rdp_texrect()
{
    _u32   a = auto_pc[auto_pc_i];
    auto_pc[auto_pc_i] = (a + 16) & 0x007fffff;   
}

void auto_fixme()
{
    auto_wrong_ucode = 1;
}

//////////////////////////////////////////////////////////////////////////////
// UCode 0                                                                  //
//////////////////////////////////////////////////////////////////////////////
void auto_rsp_uc00_cmd()
{}

void auto_rsp_uc00_displaylist()
{
    _u32 addr = segoffset2addr(auto_cmd1);
    _u32 push = (auto_cmd0 >> 16) & 0xff; 

    switch(push)
    {
        case 0:   //** push: we do a call of the dl 
            auto_pc_i++;
            if(auto_pc_i > 9)
            {
                return;
            }
            auto_pc[auto_pc_i] = addr;
            break;

        case 1:   //** no push: we jump to the dl 
            auto_pc[auto_pc_i] = addr;
            break;
        default:
            break;
    }
}

void auto_rsp_uc00_enddl()
{
    if(auto_pc_i < 0)
    {
        return;
    }

    if(auto_pc_i == 0)
    {
        auto_halt = 1;
    }
    auto_pc_i--;
}

void auto_rsp_uc00_moveword()
{
    switch(auto_cmd0 & 0xff)
    {
        case 0x06:
            auto_segment[(auto_cmd0 >> 10) & 0xf] = auto_cmd1;
            break;
        default:
            break;
    }

}

void auto_rsp_uc00_tri1()
{
    int vn;

    vn = ((auto_cmd1 >> 16) & 0xff);
    if ((vn % 10) != 0) auto_wrong_ucode = 1;
    vn = ((auto_cmd1 >>  8) & 0xff);
    if ((vn % 10) != 0) auto_wrong_ucode = 1;
    vn = ( auto_cmd1        & 0xff);
    if ((vn % 10) != 0) auto_wrong_ucode = 1;
}

//////////////////////////////////////////////////////////////////////////////
// UCode 1                                                                  //
//////////////////////////////////////////////////////////////////////////////
void auto_rsp_uc01_cmd()
{}

void auto_rsp_uc01_displaylist()
{
    _u32 addr = segoffset2addr(auto_cmd1);
    _u32 push = (auto_cmd0 >> 16) & 0xff; 

    switch(push)
    {
        case 0:   //** push: we do a call of the dl 
            auto_pc_i++;
            if(auto_pc_i > 9)
            {
                return;
            }
            auto_pc[auto_pc_i] = addr;
            break;

        case 1:   //** no push: we jump to the dl 
            auto_pc[auto_pc_i] = addr;
            break;
        default:
            break;
    }
}

void auto_rsp_uc01_enddl()
{
    if(auto_pc_i < 0)
    {
        return;
    }

    if(auto_pc_i == 0)
    {
        auto_halt = 1;
    }
    auto_pc_i--;
}

void auto_rsp_uc01_moveword()
{
    switch(auto_cmd0 & 0xff)
    {
        case 0x06:
            auto_segment[(auto_cmd0 >> 10) & 0xf] = auto_cmd1;
            break;
        default:
            break;
    }

}

void auto_rsp_uc01_tri1()
{
    int vn;
    vn = ((auto_cmd1 >> 16) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ((auto_cmd1 >>  8) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ( auto_cmd1 & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
}


void auto_rsp_uc01_tri2()
{
    int vn;
    vn = ((auto_cmd0 >> 16) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ((auto_cmd0 >>  8) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ( auto_cmd0 & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;

    vn = ((auto_cmd1 >> 16) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ((auto_cmd1 >>  8) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ( auto_cmd1 & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;

}

void auto_rsp_uc01_line3d()
{
    int vn;
    vn = ((auto_cmd0 >> 16) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ((auto_cmd0 >>  8) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ( auto_cmd0 & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;

    vn = ((auto_cmd1 >> 16) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ((auto_cmd1 >>  8) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ( auto_cmd1 & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;

}

//////////////////////////////////////////////////////////////////////////////
// UCode 2                                                                  //
//////////////////////////////////////////////////////////////////////////////
void auto_rsp_uc02_cmd()
{}

void auto_rsp_uc02_displaylist()
{
    _u32 addr = segoffset2addr(auto_cmd1);
    _u32 push = (auto_cmd0 >> 16) & 0xff; 

    switch(push)
    {
        case 0:   //** push: we do a call of the dl 
            auto_pc_i++;
            if(auto_pc_i > 9)
            {
                return;
            }
            auto_pc[auto_pc_i] = addr;
            break;

        case 1:   //** no push: we jump to the dl 
            auto_pc[auto_pc_i] = addr;
            break;
        default:
            break;
    }
}

void auto_rsp_uc02_enddl()
{
    if(auto_pc_i < 0)
    {
        return;
    }

    if(auto_pc_i == 0)
    {
        auto_halt = 1;
    }
    auto_pc_i--;
}

void auto_rsp_uc02_moveword()
{
    switch(auto_cmd0 & 0xff)
    {
        case 0x06:
            auto_segment[(auto_cmd0 >> 10) & 0xf] = auto_cmd1;
            break;
        default:
            break;
    }

}

void auto_rsp_uc02_tri1()
{
    int vn;
    vn = ((auto_cmd1 >> 16) & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;
    vn = ((auto_cmd1 >>  8) & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;
    vn = ( auto_cmd1 & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;
}


void auto_rsp_uc02_tri2()
{
    int vn;
    vn = ((auto_cmd0 >> 16) & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;
    vn = ((auto_cmd0 >>  8) & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;
    vn = ( auto_cmd0 & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;

    vn = ((auto_cmd1 >> 16) & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;
    vn = ((auto_cmd1 >>  8) & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;
    vn = ( auto_cmd1 & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;

}

//////////////////////////////////////////////////////////////////////////////
// UCode 3                                                                  //
//////////////////////////////////////////////////////////////////////////////
void auto_rsp_uc03_cmd()
{}

void auto_rsp_uc03_displaylist()
{
    _u32 addr = segoffset2addr(auto_cmd1);
    _u32 push = (auto_cmd0 >> 16) & 0xff; 

    switch(push)
    {
        case 0:   //** push: we do a call of the dl 
            auto_pc_i++;
            if(auto_pc_i > 9)
            {
                return;
            }
            auto_pc[auto_pc_i] = addr;
            break;

        case 1:   //** no push: we jump to the dl 
            auto_pc[auto_pc_i] = addr;
            break;
        default:
            break;
    }
}

void auto_rsp_uc03_enddl()
{
    if(auto_pc_i < 0)
    {
        return;
    }

    if(auto_pc_i == 0)
    {
        auto_halt = 1;
    }
    auto_pc_i--;
}

void auto_rsp_uc03_moveword()
{
    switch(auto_cmd0 & 0xff)
    {
        case 0x06:
            auto_segment[(auto_cmd0 >> 10) & 0xf] = auto_cmd1;
            break;
        default:
            break;
    }

}

void auto_rsp_uc03_tri1()
{
    int vn;
    vn = ((auto_cmd1 >> 16) & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;
    vn = ((auto_cmd1 >>  8) & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;
    vn = ( auto_cmd1 & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;
}


void auto_rsp_uc03_tri2()
{
    int vn;
    vn = ((auto_cmd0 >> 16) & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;
    vn = ((auto_cmd0 >>  8) & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;
    vn = ( auto_cmd0 & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;

    vn = ((auto_cmd1 >> 16) & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;
    vn = ((auto_cmd1 >>  8) & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;
    vn = ( auto_cmd1 & 0xff);
    if ((vn % 5) != 0) auto_wrong_ucode = 1;

}

//////////////////////////////////////////////////////////////////////////////
// UCode 4                                                                  //
//////////////////////////////////////////////////////////////////////////////
void auto_rsp_uc04_cmd()
{}

void auto_rsp_uc04_displaylist()
{
    _u32 addr = segoffset2addr(auto_cmd1);
    _u32 push = (auto_cmd0 >> 16) & 0xff; 

    switch(push)
    {
        case 0:   //** push: we do a call of the dl 
            auto_pc_i++;
            if(auto_pc_i > 9)
            {
                return;
            }
            auto_pc[auto_pc_i] = addr;
            break;

        case 1:   //** no push: we jump to the dl 
            auto_pc[auto_pc_i] = addr;
            break;
        default:
            break;
    }
}

void auto_rsp_uc04_enddl()
{
    if(auto_pc_i < 0)
    {
        return;
    }

    if(auto_pc_i == 0)
    {
        auto_halt = 1;
    }
    auto_pc_i--;
}

void auto_rsp_uc04_moveword()
{
    _u16 offset= (_u16)(auto_cmd0 & 0xffff);
    _u8 index=   (_u8)(auto_cmd0 >>16)&0xff;
    _u64 data= auto_cmd1;

    switch(index)
    {
        case 0x06:
            auto_segment[offset >> 2] = auto_cmd1 & 0xffffff;
            break;
        default:
            break;
    }

}

void auto_rsp_uc04_tri1()
{
    int vn;
    vn = ((auto_cmd0 >> 16) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ((auto_cmd0 >>  8) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ( auto_cmd0 & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
}


void auto_rsp_uc04_tri2()
{
    int vn;
    vn = ((auto_cmd0 >> 16) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ((auto_cmd0 >>  8) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ( auto_cmd0 & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;

    vn = ((auto_cmd1 >> 16) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ((auto_cmd1 >>  8) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ( auto_cmd1 & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;

}

void auto_rsp_uc04_quad()
{
    int vn;
    vn = ((auto_cmd0 >> 16) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ((auto_cmd0 >>  8) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ( auto_cmd0 & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;

    vn = ((auto_cmd1 >> 16) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ((auto_cmd1 >>  8) & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;
    vn = ( auto_cmd1 & 0xff);
    if ((vn % 2) != 0) auto_wrong_ucode = 1;

}
#include "auto_tlb.h"

//////////////////////////////////////////////////////////////////////////////
// Controll-Functions                                                       //
//////////////////////////////////////////////////////////////////////////////
int AutodetectUCode(unsigned __int32 DL_Addr)
{
    if (CheckDList(DL_Addr, 0) == 1) return 0;      //mario         div 10
    if (CheckDList(DL_Addr, 2) == 1) return 2;      //mmm           div  5  

// i think 3 and 1 are the same ...
    if (CheckDList(DL_Addr, 3) == 1) return 3;      //waveracer     div  2
    if (CheckDList(DL_Addr, 1) == 1) return 1;      //mmm           div  2

    if (CheckDList(DL_Addr, 4) == 1) return 4;      //zelda         div  2
    

    DebugBox("Cant autodetect");

    return 4;
}


int CheckDList(unsigned __int32 dwAddr, int check_ucode)
{
    _u32    a, ta;

    auto_counter = 0;
    auto_halt = 0;
    auto_pc_i = 0;
    auto_wrong_ucode = 0;
    auto_pc[auto_pc_i] = dwAddr;
    do
    {
        a = auto_pc[auto_pc_i] & 0x007fffff;
        ta = a >> 2;

    //** load next commando
        auto_cmd0 = ((_u32 *)pRDRAM)[ta + 0];
        auto_cmd1 = ((_u32 *)pRDRAM)[ta + 1];

    //** point to next instruction 
        auto_pc[auto_pc_i] = (a + 8) & 0x007fffff;   

        auto_instruction[check_ucode][auto_cmd0>>24]();

        auto_counter++;
        if (auto_counter > 10000)
        {
            auto_wrong_ucode = 1;
        }
        if (auto_wrong_ucode != 0) return 0;
    } while(!auto_halt);

    return 1;
}