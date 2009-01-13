#if !defined(__LINUX__) && !defined(__sgi)
# include <windows.h>
#else
# include "../main/winlnxdefs.h"
#endif // !__LINUX__ && !__sgi

void CRC_BuildTable();

DWORD CRC_Calculate( DWORD crc, void *buffer, DWORD count );
DWORD CRC_CalculatePalette( DWORD crc, void *buffer, DWORD count );

