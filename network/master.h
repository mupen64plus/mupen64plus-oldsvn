/*
################################################################################
================================================================================

 * Mupen64Plus - master.c
 * Copywrite 2008 orbitaldecay
 *
 * Mupen64Plus homepage: http://code.google.com/p/mupen64plus/
 * 
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *

  Functions for contacting the master server

================================================================================
################################################################################
*/

#include <stdint.h>
#include <SDL_net.h>

// Master Server Defines (Packet Types)
#define FIND_GAMES      00
#define GAME_LIST	01
#define OPEN_GAME       02
#define GAME_DESC	03
#define KEEP_ALIVE      04 
#define FIND_MD5        06
#define MD5_LIST        07

typedef struct TMD5ListNode {
    unsigned char    md5[16];
    struct TMD5ListNode *next;
} MD5ListNode;

typedef struct THostListNode {
    uint32_t           host;
    uint16_t           port;
    struct THostListNode *next;
} HostListNode;

extern IPaddress g_Game_Master;

void          MasterServerAddToList(char *master_server);
int           MasterServerCreateGame(unsigned char md5[16], int local_port);
void          MasterServerCloseGame();
MD5ListNode  *MasterServerGetMD5List();
HostListNode *MasterServerFindGames(unsigned char md5[16]);

HostListNode *GetFirstMasterServer();
HostListNode *GetNextHost(HostListNode *node);
HostListNode *CombineHostLists(HostListNode *list1, HostListNode *list2);
void          FreeHostList(HostListNode *list);
MD5ListNode  *GetNextMD5(MD5ListNode *node);
MD5ListNode  *CombineMD5Lists(MD5ListNode *list1, MD5ListNode *list2);
void          FreeMD5List(MD5ListNode *list);
int           sendNATPunchRequest(uint32_t dhost, uint16_t dport, uint32_t thost, uint16_t tport);

// Internal Functions
static void         *KeepAliveThread( void *_arg );
static long int      timeElapsed(unsigned char arm);
static int           masterServerOpenGame  (uint32_t master_server, uint16_t master_port, unsigned char md5[16], uint16_t local_port);
static int           masterServerKeepAlive (uint32_t master_server, uint16_t master_port, uint16_t game_id);
static MD5ListNode  *masterServerGetMD5List(uint32_t master_server, uint16_t master_port, int *status);
static HostListNode *masterServerFindGames(uint32_t master_server, uint16_t master_port, unsigned char md5[16], int *status);
