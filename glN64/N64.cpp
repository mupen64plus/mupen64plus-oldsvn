#if !defined(__LINUX__) && !defined(__sgi)
# include <windows.h>
#else
# include "../main/winlnxdefs.h"
#endif // !__LINUX__ && !__sgi
#include "N64.h"
#include "Types.h"

u8 *DMEM;
u8 *IMEM;
u64 TMEM[512];
u8 *RDRAM;
u32 RDRAMSize;

N64Regs REG;

