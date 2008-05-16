//
// SDL Input for TASinput
// written by nmn
//

#ifndef _DEFSDL_H_INCLUDED__
#define _DEFSDL_H_INCLUDED__

#define NUMBER_OF_CONTROLS  4
#define NUMBER_OF_BUTTONS   25

#define INPUT_TYPE_NOT_USED     0
#define INPUT_TYPE_KEY_BUT      1
#define INPUT_TYPE_JOY_BUT      2
#define INPUT_TYPE_JOY_AXIS     3
#define INPUT_TYPE_JOY_POV      4

#define IDT_TIMER3 3

typedef struct 
{
    BYTE Device;
    BYTE type;
    BYTE vkey;
    DWORD button;
} INPUT;

typedef struct 
{
    TCHAR szName[16];
    BYTE NDevices;
    DWORD Devices[MAX_DEVICES];
    BOOL bActive;
    BOOL bMemPak;
    BYTE SensMax;
    BYTE SensMin;
    INPUT Input[NUMBER_OF_BUTTONS];
} DEFCONTROLLER;

extern DEFCONTROLLER Controller[NUMBER_OF_CONTROLS];

extern HINSTANCE g_hInstance;

void WINAPI InitializeAndCheckDevices(HWND hMainWindow);
BOOL WINAPI CheckForDeviceChange(HKEY hKey);

#endif