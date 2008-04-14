#ifdef USEWIN32
# include <windows.h>
#endif
#ifdef USEPOSIX
# include "../main/wintypes.h"
#endif // __LINUX__

void CRC_BuildTable();

DWORD CRC_Calculate( DWORD crc, void *buffer, DWORD count );
DWORD CRC_CalculatePalette( DWORD crc, void *buffer, DWORD count );
