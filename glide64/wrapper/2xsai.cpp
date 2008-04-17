#ifdef _WIN32
#include <windows.h>
#else // _WIN32
#include "../winlnxdefs.h"
#endif // _WIN32

static __inline int GetResult1( DWORD A, DWORD B, DWORD C, DWORD D, DWORD E )
{
    int x = 0;
    int y = 0;
    int r = 0;

    if (A == C) x += 1; else if (B == C) y += 1;
    if (A == D) x += 1; else if (B == D) y += 1;
    if (x <= 1) r += 1; 
    if (y <= 1) r -= 1;

    return r;
}

static __inline WORD GetResult1_16( WORD A, WORD B, WORD C, WORD D, WORD E )
{
    WORD x = 0;
    WORD y = 0;
    WORD r = 0;

    if (A == C) x += 1; else if (B == C) y += 1;
    if (A == D) x += 1; else if (B == D) y += 1;
    if (x <= 1) r += 1; 
    if (y <= 1 && r>0) r -= 1;

    return r;
}

static __inline int GetResult2( DWORD A, DWORD B, DWORD C, DWORD D, DWORD E) 
{
    int x = 0; 
    int y = 0;
    int r = 0;

    if (A == C) x += 1; else if (B == C) y += 1;
    if (A == D) x += 1; else if (B == D) y += 1;
    if (x <= 1) r -= 1; 
    if (y <= 1) r += 1;

    return r;
}

static __inline WORD GetResult2_16( WORD A, WORD B, WORD C, WORD D, WORD E) 
{
    WORD x = 0; 
    WORD y = 0;
    WORD r = 0;

    if (A == C) x += 1; else if (B == C) y += 1;
    if (A == D) x += 1; else if (B == D) y += 1;
    if (x <= 1 && r>0 ) r -= 1; 
    if (y <= 1) r += 1;

    return r;
}


static __inline int GetResult( DWORD A, DWORD B, DWORD C, DWORD D )
{
    int x = 0; 
    int y = 0;
    int r = 0;

    if (A == C) x += 1; else if (B == C) y += 1;
    if (A == D) x += 1; else if (B == D) y += 1;
    if (x <= 1) r += 1; 
    if (y <= 1) r -= 1;

    return r;
}

static __inline WORD GetResult_16( WORD A, WORD B, WORD C, WORD D )
{
    WORD x = 0; 
    WORD y = 0;
    WORD r = 0;

    if (A == C) x += 1; else if (B == C) y += 1;
    if (A == D) x += 1; else if (B == D) y += 1;
    if (x <= 1) r += 1; 
    if (y <= 1 && r>0 ) r -= 1;

    return r;
}


static __inline DWORD INTERPOLATE( DWORD A, DWORD B)
{
    if (A != B)
        return  ((A & 0xFEFEFEFE) >> 1) + 
                ((B & 0xFEFEFEFE) >> 1) |
                (A & B & 0x01010101);
    else
        return A;
}

static __inline WORD INTERPOLATE_16( WORD A, WORD B)
{
    if (A != B)
        return  ((A & 0xFEFE) >> 1) + 
                ((B & 0xFEFE) >> 1) |
                (A & B & 0x0101);
    else
        return A;
}


static __inline DWORD Q_INTERPOLATE( DWORD A, DWORD B, DWORD C, DWORD D)
{
    DWORD x =   ((A & 0xFCFCFCFC) >> 2) +
                ((B & 0xFCFCFCFC) >> 2) +
                ((C & 0xFCFCFCFC) >> 2) +
                ((D & 0xFCFCFCFC) >> 2);
    DWORD y =   (((A & 0x03030303) +
                (B & 0x03030303) +
                (C & 0x03030303) +
                (D & 0x03030303)) >> 2) & 0x03030303;
    return x | y;
}

static __inline WORD Q_INTERPOLATE_16( WORD A, WORD B, WORD C, WORD D)
{
    WORD x =    ((A & 0xFCFC) >> 2) +
                ((B & 0xFCFC) >> 2) +
                ((C & 0xFCFC) >> 2) +
                ((D & 0xFCFC) >> 2);
    WORD y =    (((A & 0x0303) +
                (B & 0x0303) +
                (C & 0x0303) +
                (D & 0x0303)) >> 2) & 0x0303;
    return x | y;
}


void Super2xSaI( DWORD *srcPtr, DWORD *destPtr, DWORD width, DWORD height, DWORD pitch)
{
    DWORD destWidth = width << 1;
#ifdef _WIN32
    DWORD destHeight = height << 1;
#endif // _WIN32

    DWORD color4, color5, color6;
    DWORD color1, color2, color3;
    DWORD colorA0, colorA1, colorA2, colorA3;
    DWORD colorB0, colorB1, colorB2, colorB3;
    DWORD colorS1, colorS2;
    DWORD product1a, product1b, product2a, product2b;

    int row0, row1, row2, row3;
    int col0, col1, col2, col3;

    WORD y, x;

    for (y = 0; y < height; y++)
    {
        if (y > 0)
        {
            row0 = width;
            row0 = -row0;
        }
        else
            row0 = 0;

        row1 = 0;

        if (y < height - 1)
        {
            row2 = width;

            if (y < height - 2) 
                row3 = width << 1;
            else
                row3 = width;
        }
        else
        {
            row2 = 0;
            row3 = 0;
        }

        for (x = 0; x < width; x++)
        {
//--------------------------------------- B0 B1 B2 B3
//                                         4  5  6 S2
//                                         1  2  3 S1
//                                        A0 A1 A2 A3
            if (x > 0)
                col0 = -1;
            else
                col0 = 0;

            col1 = 0;

            if (x < width - 1)
            {
                col2 = 1;

                if (x < width - 2) 
                    col3 = 2;
                else
                    col3 = 1;
            }
            else
            {
                col2 = 0;
                col3 = 0;
            }

            colorB0 = *(srcPtr + col0 + row0);
            colorB1 = *(srcPtr + col1 + row0);
            colorB2 = *(srcPtr + col2 + row0);
            colorB3 = *(srcPtr + col3 + row0);

            color4 = *(srcPtr + col0 + row1);
            color5 = *(srcPtr + col1 + row1);
            color6 = *(srcPtr + col2 + row1);
            colorS2 = *(srcPtr + col3 + row1);

            color1 = *(srcPtr + col0 + row2);
            color2 = *(srcPtr + col1 + row2);
            color3 = *(srcPtr + col2 + row2);
            colorS1 = *(srcPtr + col3 + row2);

            colorA0 = *(srcPtr + col0 + row3);
            colorA1 = *(srcPtr + col1 + row3);
            colorA2 = *(srcPtr + col2 + row3);
            colorA3 = *(srcPtr + col3 + row3);

//--------------------------------------
            if (color2 == color6 && color5 != color3)
                product2b = product1b = color2;
            else if (color5 == color3 && color2 != color6)
                product2b = product1b = color5;
            else if (color5 == color3 && color2 == color6)
            {
                int r = 0;

                r += GetResult (color6, color5, color1, colorA1);
                r += GetResult (color6, color5, color4, colorB1);
                r += GetResult (color6, color5, colorA2, colorS1);
                r += GetResult (color6, color5, colorB2, colorS2);

                if (r > 0)
                    product2b = product1b = color6;
                else if (r < 0)
                    product2b = product1b = color5;
                else
                    product2b = product1b = INTERPOLATE (color5, color6);
            }
            else
            {

                if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
                    product2b = Q_INTERPOLATE (color3, color3, color3, color2);
                else if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
                    product2b = Q_INTERPOLATE (color2, color2, color2, color3);
                else
                    product2b = INTERPOLATE (color2, color3);

                if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
                    product1b = Q_INTERPOLATE (color6, color6, color6, color5);
                else if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
                    product1b = Q_INTERPOLATE (color6, color5, color5, color5);
                else
                    product1b = INTERPOLATE (color5, color6);
            }

            if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
                product2a = INTERPOLATE (color2, color5);
            else if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
                product2a = INTERPOLATE(color2, color5);
            else
                product2a = color2;

            if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
                product1a = INTERPOLATE (color2, color5);
            else if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
                product1a = INTERPOLATE(color2, color5);
            else
                product1a = color5;


            destPtr[0] = product1a;
            destPtr[1] = product1b;
            destPtr[destWidth] = product2a;
            destPtr[destWidth + 1] = product2b;

            srcPtr++;
            destPtr += 2;
        }
        srcPtr += (pitch-width);
        destPtr += (((pitch-width)<<1)+(pitch<<1));
    }
}


void Super2xSaI_16( WORD *srcPtr, WORD *destPtr, DWORD width, DWORD height, DWORD pitch)
{
    DWORD destWidth = width << 1;
#ifdef _WIN32
    DWORD destHeight = height << 1;
#endif // _WIN32

    WORD color4, color5, color6;
    WORD color1, color2, color3;
    WORD colorA0, colorA1, colorA2, colorA3;
    WORD colorB0, colorB1, colorB2, colorB3;
    WORD colorS1, colorS2;
    WORD product1a, product1b, product2a, product2b;

    int row0, row1, row2, row3;
    int col0, col1, col2, col3;

    WORD y, x;

    for (y = 0; y < height; y++)
    {
        if (y > 0)
        {
            row0 = width;
            row0 = -row0;
        }
        else
            row0 = 0;

        row1 = 0;

        if (y < height - 1)
        {
            row2 = width;

            if (y < height - 2) 
                row3 = width << 1;
            else
                row3 = width;
        }
        else
        {
            row2 = 0;
            row3 = 0;
        }

        for (x = 0; x < width; x++)
        {
//--------------------------------------- B0 B1 B2 B3
//                                         4  5  6 S2
//                                         1  2  3 S1
//                                        A0 A1 A2 A3
            if (x > 0)
                col0 = -1;
            else
                col0 = 0;

            col1 = 0;

            if (x < width - 1)
            {
                col2 = 1;

                if (x < width - 2) 
                    col3 = 2;
                else
                    col3 = 1;
            }
            else
            {
                col2 = 0;
                col3 = 0;
            }

            colorB0 = *(srcPtr + col0 + row0);
            colorB1 = *(srcPtr + col1 + row0);
            colorB2 = *(srcPtr + col2 + row0);
            colorB3 = *(srcPtr + col3 + row0);

            color4 = *(srcPtr + col0 + row1);
            color5 = *(srcPtr + col1 + row1);
            color6 = *(srcPtr + col2 + row1);
            colorS2 = *(srcPtr + col3 + row1);

            color1 = *(srcPtr + col0 + row2);
            color2 = *(srcPtr + col1 + row2);
            color3 = *(srcPtr + col2 + row2);
            colorS1 = *(srcPtr + col3 + row2);

            colorA0 = *(srcPtr + col0 + row3);
            colorA1 = *(srcPtr + col1 + row3);
            colorA2 = *(srcPtr + col2 + row3);
            colorA3 = *(srcPtr + col3 + row3);

//--------------------------------------
            if (color2 == color6 && color5 != color3)
                product2b = product1b = color2;
            else if (color5 == color3 && color2 != color6)
                product2b = product1b = color5;
            else if (color5 == color3 && color2 == color6)
            {
                int r = 0;

                r += GetResult_16 (color6, color5, color1, colorA1);
                r += GetResult_16 (color6, color5, color4, colorB1);
                r += GetResult_16 (color6, color5, colorA2, colorS1);
                r += GetResult_16 (color6, color5, colorB2, colorS2);

                if (r > 0)
                    product2b = product1b = color6;
                else if (r < 0)
                    product2b = product1b = color5;
                else
                    product2b = product1b = INTERPOLATE_16 (color5, color6);
            }
            else
            {

                if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
                    product2b = Q_INTERPOLATE_16 (color3, color3, color3, color2);
                else if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
                    product2b = Q_INTERPOLATE_16 (color2, color2, color2, color3);
                else
                    product2b = INTERPOLATE_16 (color2, color3);

                if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
                    product1b = Q_INTERPOLATE_16 (color6, color6, color6, color5);
                else if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
                    product1b = Q_INTERPOLATE_16 (color6, color5, color5, color5);
                else
                    product1b = INTERPOLATE_16 (color5, color6);
            }

            if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
                product2a = INTERPOLATE_16 (color2, color5);
            else if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
                product2a = INTERPOLATE_16(color2, color5);
            else
                product2a = color2;

            if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
                product1a = INTERPOLATE_16 (color2, color5);
            else if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
                product1a = INTERPOLATE_16(color2, color5);
            else
                product1a = color5;


            destPtr[0] = product1a;
            destPtr[1] = product1b;
            destPtr[destWidth] = product2a;
            destPtr[destWidth + 1] = product2b;

            srcPtr++;
            destPtr += 2;
        }
        srcPtr += (pitch-width);
        destPtr += (((pitch-width)<<1)+(pitch<<1));
    }
}


/*void SuperEagle(uint8 *srcPtr, uint32 srcPitch,
         uint8 *deltaPtr,
         BITMAP *dstBitmap, int width, int height)
{
    uint32 dstPitch = dstBitmap->w * 2;
    uint32 line;
    uint32 x_offset;
    uint32 dP;
    uint16 *bP;
    uint16 *xP;
    uint32 inc_bP;
    line = (dstBitmap->h - height * 2) >> 1;

#ifdef MMX
    if (cpu_mmx && width != 512)
    {
        x_offset = (dstBitmap->w - width * 2);
    for (height; height; height-=1)
    {
        bP = (uint16 *) srcPtr;
            xP = (uint16 *) deltaPtr;
        dP = bmp_write_line (dstBitmap, line) + x_offset;
            _2xSaISuperEagleLine  ((uint8 *) bP, (uint8 *) xP, srcPitch, width, dP, dstPitch, dstBitmap->seg);
            line += 2;
        srcPtr += srcPitch;
            deltaPtr += srcPitch;
        }
    }
    else
#endif
    {
        x_offset = (dstBitmap->w - width * 2);
        inc_bP = 1;

        _farsetsel (dstBitmap->seg);
        uint32 Nextline = srcPitch >> 1;

        for (height; height; height-=1)
    {
        bP = (uint16 *) srcPtr;
            xP = (uint16 *) deltaPtr;
        dP = bmp_write_line (dstBitmap, line) + x_offset;
            for (uint32 finish = width; finish; finish -= inc_bP )
            {

                uint32 color4, color5, color6;
                uint32 color1, color2, color3;
                uint32 colorA0, colorA1, colorA2, colorA3,
                       colorB0, colorB1, colorB2, colorB3,
                       colorS1, colorS2;
                uint32 product1a, product1b,
                       product2a, product2b;

                colorB1 = *(bP- Nextline);
                colorB2 = *(bP- Nextline + 1);

                color4 = *(bP - 1);
                color5 = *(bP);
                color6 = *(bP + 1);
                colorS2 = *(bP + 2);

                color1 = *(bP + Nextline - 1);
                color2 = *(bP + Nextline);
                color3 = *(bP + Nextline + 1);
                colorS1 = *(bP + Nextline + 2);

                colorA1 = *(bP + Nextline + Nextline);
                colorA2 = *(bP + Nextline + Nextline + 1);

                //--------------------------------------
                if (color2 == color6 && color5 != color3)
                {
                   product1b = product2a = color2;
                   if ((color1 == color2) ||
                       (color6 == colorB2))
                   {
                       product1a = INTERPOLATE (color2, color5);
                       product1a = INTERPOLATE (color2, product1a);
//                       product1a = color2;
                   }
                   else
                   {
                      product1a = INTERPOLATE (color5, color6);
                   }

           if ((color6 == colorS2) ||
                       (color2 == colorA1))
                   {
                       product2b = INTERPOLATE (color2, color3);
                       product2b = INTERPOLATE (color2, product2b);
//                       product2b = color2;
                   }
                   else
                   {
                      product2b = INTERPOLATE (color2, color3);
                   }
                }
                else
                if (color5 == color3 && color2 != color6)
                {
                   product2b = product1a = color5;

                   if ((colorB1 == color5) ||
                       (color3 == colorS1))
                   {
                       product1b = INTERPOLATE (color5, color6);
                       product1b = INTERPOLATE (color5, product1b);
//                       product1b = color5;
                   }
                   else
                   {
                      product1b = INTERPOLATE (color5, color6);
                   }

           if ((color3 == colorA2) ||
                       (color4 == color5))
                   {
                       product2a = INTERPOLATE (color5, color2);
                       product2a = INTERPOLATE (color5, product2a);
//                       product2a = color5;
                   }
                   else
                   {
                      product2a = INTERPOLATE (color2, color3);
                   }

                }
                else
                if (color5 == color3 && color2 == color6)
                {
                   register int r = 0;

                   r += GetResult (color6, color5, color1, colorA1);
                   r += GetResult (color6, color5, color4, colorB1);
                   r += GetResult (color6, color5, colorA2, colorS1);
                   r += GetResult (color6, color5, colorB2, colorS2);

                   if (r > 0)
                   {
                      product1b = product2a = color2;
                      product1a = product2b = INTERPOLATE (color5, color6);
                   }
                   else
                   if (r < 0)
                   {
                      product2b = product1a = color5;
                      product1b = product2a = INTERPOLATE (color5, color6);
                   }
                   else
                   {
                      product2b = product1a = color5;
                      product1b = product2a = color2;
                   }
                }
                else
                {
                      product2b = product1a = INTERPOLATE (color2, color6);
                      product2b = Q_INTERPOLATE (color3, color3, color3, product2b);
                      product1a = Q_INTERPOLATE (color5, color5, color5, product1a);

                      product2a = product1b = INTERPOLATE (color5, color3);
                      product2a = Q_INTERPOLATE (color2, color2, color2, product2a);
                      product1b = Q_INTERPOLATE (color6, color6, color6, product1b);

//            product1a = color5;
//            product1b = color6;
//            product2a = color2;
//            product2b = color3;
                }
                    product1a = product1a | (product1b << 16);
                    product2a = product2a | (product2b << 16);

                    _farnspokel (dP, product1a);
                    _farnspokel (dP + dstPitch, product2a);
                    *xP = color5;

                    bP += inc_bP;
                    xP += inc_bP;
                    dP += 4;
                }//end of for ( finish= width etc..)

            line += 2;
            srcPtr += srcPitch;
            deltaPtr += srcPitch;
    }; //endof: for (height; height; height--)
    }
}


*/
void _2xSaI( DWORD *srcPtr, DWORD *destPtr, WORD width, WORD height, BOOL clampS, BOOL clampT )
{
    WORD destWidth = width << 1;
#ifdef _WIN32
    WORD destHeight = height << 1;
#endif // _WIN32

    DWORD colorA, colorB, colorC, colorD,
          colorE, colorF, colorG, colorH,
          colorI, colorJ, colorK, colorL,
          colorM, colorN, colorO, colorP;
    DWORD product, product1, product2;

    int row0, row1, row2, row3;
    int col0, col1, col2, col3;

    WORD y, x;

    for (y = 0; y < height; y++)
    {
        if (y > 0)
            row0 = -width;
        else
            row0 = clampT ? 0 : (height - 1) * width;

        row1 = 0;

        if (y < height - 1)
        {
            row2 = width;

            if (y < height - 2) 
                row3 = width << 1;
            else
                row3 = clampT ? width : -y * width;
        }
        else
        {
            row2 = clampT ? 0 : -y * width;
            row3 = clampT ? 0 : (1 - y) * width;
        }

        for (x = 0; x < width; x++)
        {
            if (x > 0)
                col0 = -1;
            else
                col0 = clampS ? 0 : width - 1;

            col1 = 0;

            if (x < width - 1)
            {
                col2 = 1;

                if (x < width - 2) 
                    col3 = 2;
                else
                    col3 = clampS ? 1 : -x;
            }
            else
            {
                col2 = clampS ? 0 : -x;
                col3 = clampS ? 0 : 1 - x;
            }

//---------------------------------------
// Map of the pixels:                    I|E F|J
//                                       G|A B|K
//                                       H|C D|L
//                                       M|N O|P
            colorI = *(srcPtr + col0 + row0);
            colorE = *(srcPtr + col1 + row0);
            colorF = *(srcPtr + col2 + row0);
            colorJ = *(srcPtr + col3 + row0);

            colorG = *(srcPtr + col0 + row1);
            colorA = *(srcPtr + col1 + row1);
            colorB = *(srcPtr + col2 + row1);
            colorK = *(srcPtr + col3 + row1);

            colorH = *(srcPtr + col0 + row2);
            colorC = *(srcPtr + col1 + row2);
            colorD = *(srcPtr + col2 + row2);
            colorL = *(srcPtr + col3 + row2);

            colorM = *(srcPtr + col0 + row3);
            colorN = *(srcPtr + col1 + row3);
            colorO = *(srcPtr + col2 + row3);
            colorP = *(srcPtr + col3 + row3);

            if ((colorA == colorD) && (colorB != colorC))
            {
                if ( ((colorA == colorE) && (colorB == colorL)) ||
                    ((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ)) )
                    product = colorA;
                else
                    product = INTERPOLATE(colorA, colorB);

                if (((colorA == colorG) && (colorC == colorO)) ||
                    ((colorA == colorB) && (colorA == colorH) && (colorG != colorC) && (colorC == colorM)) )
                    product1 = colorA;
                else
                    product1 = INTERPOLATE(colorA, colorC);

                product2 = colorA;
            }
            else if ((colorB == colorC) && (colorA != colorD))
            {
                if (((colorB == colorF) && (colorA == colorH)) ||
                    ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI)) )
                    product = colorB;
                else
                    product = INTERPOLATE(colorA, colorB);
 
                if (((colorC == colorH) && (colorA == colorF)) ||
                    ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI)) )
                    product1 = colorC;
                else
                    product1 = INTERPOLATE(colorA, colorC);
                product2 = colorB;
            }
            else if ((colorA == colorD) && (colorB == colorC))
            {
                if (colorA == colorB)
                {
                    product = colorA;
                    product1 = colorA;
                    product2 = colorA;
                }
                else
                {
                    int r = 0;
                    product1 = INTERPOLATE(colorA, colorC);
                    product = INTERPOLATE(colorA, colorB);

                    r += GetResult1 (colorA, colorB, colorG, colorE, colorI);
                    r += GetResult2 (colorB, colorA, colorK, colorF, colorJ);
                    r += GetResult2 (colorB, colorA, colorH, colorN, colorM);
                    r += GetResult1 (colorA, colorB, colorL, colorO, colorP);

                    if (r > 0)
                        product2 = colorA;
                    else if (r < 0)
                        product2 = colorB;
                    else
                        product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);
                }
            }
            else
            {
                product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);

                if ((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ))
                    product = colorA;
                else if ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI))
                    product = colorB;
                else
                    product = INTERPOLATE(colorA, colorB);

                if ((colorA == colorB) && (colorA == colorH) && (colorG != colorC) && (colorC == colorM))
                    product1 = colorA;
                else if ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI))
                    product1 = colorC;
                else
                    product1 = INTERPOLATE(colorA, colorC);
            }

            destPtr[0] = colorA;
            destPtr[1] = product;
            destPtr[destWidth] = product1;
            destPtr[destWidth + 1] = product2;

            srcPtr++;
            destPtr += 2;
        }
        destPtr += destWidth;
    }
}
