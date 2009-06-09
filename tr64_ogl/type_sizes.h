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

#ifndef __TYPE_SIZES_H
#define __TYPE_SIZES_H

/* these are (very) often used */
#ifdef WIN32
# include <windows.h>
# define _u64 unsigned __int64   /* 64 bit */
# define _u32 unsigned __int32   /* 16 bit */
# define _u16 unsigned __int16   /* 16 bit */
# define _u8  unsigned __int8    /*  8 bit */
# define _s64          __int64   /* 64 bit */
# define _s32          __int32   /* 16 bit */
# define _s16          __int16   /* 16 bit */
# define _s8           __int8    /*  8 bit */
#else
# include <stdint.h>
# define _u64 uint64_t
# define _u32 uint32_t
# define _u16 uint16_t
# define _u8   uint8_t
# define _s64 int64_t
# define _s32 int32_t
# define _s16 int16_t
# define _s8   int8_t
#endif /* WIN32 */

#endif