#ifndef WINLNXDEFS_H
#define WINLNXDEFS_H

typedef unsigned int BOOL;
typedef unsigned short WORD;
typedef unsigned int DWORD, *LPDWORD;
typedef unsigned char BYTE, CHAR, TCHAR, *LPBYTE;
typedef unsigned int UINT, uint, ULONG;
typedef int LONG;
typedef void VOID, *LPVOID;
typedef float FLOAT;

#define __int16 short
#define __int32 int
#define __int64 long long

typedef int HWND;
typedef void* HBITMAP;
typedef char* LPCTSTR;

#define LPCSTR char*

typedef struct
{
   int top;
   int bottom;
   int right;
   int left;
} RECT;

typedef struct _COORDRECT
{
   LONG x1,y1;
   LONG x2,y2;
} COORDRECT;

#define __declspec(dllexport)
#define _cdecl
#define __cdecl

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define MAX_PATH PATH_MAX
#define _MAX_PATH PATH_MAX

typedef DWORD COLOR;
typedef int SURFFORMAT;

#define SURFFMT_A8R8G8B8 21

#define COLOR_RGBA(r,g,b,a) (((r&0xFF)<<16) | ((g&0xFF)<<8) | ((b&0xFF)<<0) | ((a&0xFF)<<24))
#define CONST const


typedef struct tagBITMAPINFOHEADER
{
   DWORD biSize;
   LONG biWidth;
   LONG biHeight;
   WORD biPlanes;
   WORD biBitCount;
   DWORD biCompression;
   DWORD biSizeImage;
   LONG biXPelsPerMeter;
   LONG biYPelsPerMeter;
   DWORD biClrUsed;
   DWORD biClrImportant;
}  __attribute__ ((packed)) BITMAPINFOHEADER;

typedef struct tagRGBQUAD
{
   BYTE rgbBlue;
   BYTE rgbGreen;
   BYTE rgbRed;
   BYTE rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO
{
   BITMAPINFOHEADER bmiHeader;
   RGBQUAD bmiColors[1];
} BITMAPINFO;

typedef enum _IMAGE_FILEFORMAT 
{
   XIFF_BMP = 0,
     XIFF_JPG = 1,
     XIFF_TGA = 2,
     XIFF_PNG = 3,
     XIFF_DDS = 4,
     XIFF_PPM = 5,
     XIFF_DIB = 6,
     XIFF_HDR = 7,
     XIFF_PFM = 8,
     XIFF_FORCE_DWORD = 0x7fffffff
} IMAGE_FILEFORMAT;

typedef struct _IMAGE_INFO
{
   UINT Width;
   UINT Height;
   UINT Depth;
   UINT MipLevels;
   SURFFORMAT Format;
   IMAGE_FILEFORMAT ImageFileFormat;
} IMAGE_INFO;


typedef struct tagBITMAPFILEHEADER
{
   WORD    bfType; 
   DWORD   bfSize; 
   WORD    bfReserved1; 
   WORD    bfReserved2; 
   DWORD   bfOffBits; 
} __attribute__ ((packed)) BITMAPFILEHEADER, *PBITMAPFILEHEADER;

#define BI_RGB 0

void OutputDebugString(LPCTSTR lpOutputString);

typedef enum _BLEND 
{
   BLEND_ZERO = 1,
     BLEND_ONE = 2,
     BLEND_SRCCOLOR = 3,
     BLEND_INVSRCCOLOR = 4,
     BLEND_SRCALPHA = 5,
     BLEND_INVSRCALPHA = 6,
     BLEND_DESTALPHA = 7,
     BLEND_INVDESTALPHA = 8,
     BLEND_DESTCOLOR = 9,
     BLEND_INVDESTCOLOR = 10,
     BLEND_SRCALPHASAT = 11,
     BLEND_BOTHSRCALPHA = 12,
     BLEND_BOTHINVSRCALPHA = 13,
     BLEND_BLENDFACTOR = 14,
     BLEND_INVBLENDFACTOR = 15,
     BLEND_FORCE_DWORD = 0x7fffffff
} BLEND;

#endif // _WINLNXDEFS_H

