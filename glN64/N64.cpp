#ifdef USEWIN32
# include <windows.h>
#endif
#ifdef USEPOSIX
# include "../main/wintypes.h"
#endif
#include "N64.h"
#include "Types.h"

u8 *DMEM;
u8 *IMEM;
u64 TMEM[512];
u8 *RDRAM;
u32 RDRAMSize;

N64Regs REG;
