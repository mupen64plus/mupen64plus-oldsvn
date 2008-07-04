/**
 * Mupen64 - master_server.c
 * Copyright (C) 2008 orbitaldecay
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
**/

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <SDL_net.h>

#define SERVER_VER     "1.0"
#define MAX_PACKET	1470  // In bytes, the largest packet permitted by the protocol
#define MAX_GAME_DESC   32768 // The highest available game descriptor (max is 65536)
#define CLEAN_FREQ      60    // In seconds, how often free_timed_out_game_desc() is called
#define PROTOCOL_ID     "M+"

#define FIND_GAMES      00
#define GAME_LIST	01
#define OPEN_GAME       02
#define GAME_DESC	03
#define KEEP_ALIVE      04

#define FIND_MD5        06
#define MD5_LIST        07

#define ELAPSED(dt)          (int)(((dt) - ((dt) % 3600)) / 3600), (int)((((dt) - ((dt) % 60)) / 60) % 60), (int)((dt) % 60)
// Thanks to DarkJeztr for this little biddy
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define GET_IP(ip)      ((unsigned char*)&(ip))[3],((unsigned char*)&(ip))[2], \
                            ((unsigned char*)&(ip))[1],((unsigned char*)&(ip))[0]
#else
    #define GET_IP(ip)      ((unsigned char*)&(ip))[0],((unsigned char*)&(ip))[1], \
                            ((unsigned char*)&(ip))[2],((unsigned char*)&(ip))[3]
#endif

typedef unsigned char md5_byte_t;

typedef struct GameEntry_t {
  uint32_t                     host; // Network Byte Order
  uint16_t                     port;
  unsigned char                keep_alive;
  struct GameEntry_t          *next, *prev;
} GameEntry;

typedef struct MD5Entry_t {
  md5_byte_t             md5_checksum[16];
  struct GameEntry_t    *first_game_entry;
  struct MD5Entry_t *next;
} MD5Entry;


// Primary Functions
void        process_packet(UDPpacket *packet);
void *      track_malloc(int size);
void        track_free(void *ptr, int bytes);

// Signal callbacks
extern void interupt_pause(int value);
extern void alarm_func(int sig);

// Game Descriptor Functions
void        remove_game_desc(int n);
int         get_free_game_desc();
int         find_game_desc_by_host(uint32_t host);
void        free_timed_out_game_desc();

// UDP Send Data Functions
void        send_game_descriptor(uint32_t host, uint16_t port, int gameDesc);
void        send_md5_list(uint32_t host, uint16_t port);
void        send_game_list(uint32_t host, uint16_t port, md5_byte_t md5_checksum[16]);

// Linked List Functions
void        add_game_entry_node(GameEntry *newGameEntry, md5_byte_t md5_checksum[16]);
MD5Entry *  add_md5_node(md5_byte_t md5_checksum[16]);
void        remove_md5_node(MD5Entry *deadNode);
MD5Entry *  find_md5_node(md5_byte_t md5_checksum[16]);
void        flush_game_entry_list(MD5Entry* md5_node);
void        clean_md5_list();

// Global Variables
time_t         g_StartTime;
int            g_QuitMainLoop = 0;
int            g_GameCount = 0;
int            g_BlocksAllocated = 0;
uint32_t       g_BytesAllocated = 0;
unsigned short g_ServerPort;
GameEntry     *g_GameList[MAX_GAME_DESC];
UDPsocket      g_ListenSock;
MD5Entry      *g_MD5List = NULL;

void print_md5(md5_byte_t md5[16]) {
  int n;
  for (n = 0; n < 16; n++) {
     printf("%x", md5[n]);
  }
}
void print_debug() {
      int n;
      MD5Entry      *temp_md5_entry = g_MD5List;
      GameEntry     *temp_game_entry;
      printf("By MD5:\n");
      while (temp_md5_entry) {
         print_md5(temp_md5_entry->md5_checksum);
         printf("\n");
         temp_game_entry = temp_md5_entry->first_game_entry->next;
         while (temp_game_entry->next) {
              printf("   %d.%d.%d.%d:%d\n", GET_IP(temp_game_entry->host), temp_game_entry->port);
              temp_game_entry = temp_game_entry->next;
         }           
         temp_md5_entry = temp_md5_entry->next;
      }       
      printf("\nBy Game Descriptor:\n");
      for (n = 0; n < MAX_GAME_DESC; n++) {
         if (g_GameList[n]) printf("%d. %d.%d.%d.%d:%d this %p next %p prev %p\n", n, GET_IP(g_GameList[n]->host), g_GameList[n]->port, g_GameList[n], g_GameList[n]->next, g_GameList[n]->prev);
      }
}

// ================================================================================
// == main() and processPacket() ==================================================
// ================================================================================

/* --------------------------------------------------------------------------------

    main()

   --------------------------------------------------------------------------------
*/

int main(int argc, char **argv) {
  UDPpacket *recvPacket;
  int retValue, n, minorVersion, majorVersion;
  MD5Entry *temp_md5_entry;

  if (argc != 2) {
       printf("Usage: master_server <port>\n");
       return 0;
  } else {
       g_ServerPort = atoi(argv[1]);
  }

  g_StartTime = time(NULL);
  memset(&g_GameList, 0, sizeof(g_GameList));
  printf("*********************************************\n");
  printf("**                                         **\n");
  printf("**   /~ Mupen64Plus Master Server %s ~\\   **\n", SERVER_VER);
  printf("**           orbitaldecay 2008             **\n");
  printf("**                                         **\n");
  printf("*********************************************\n");

  if (signal(SIGINT, SIG_IGN) != SIG_IGN) {
      if (signal(SIGINT, interupt_pause) == SIG_ERR) {
          fprintf(stderr, "signal(SIGINT): Error %d\n", errno);
          return 1;
      }
  }

  if (signal(SIGALRM, alarm_func) == SIG_ERR) {
      fprintf(stderr, "signal(SIGALRM): Error %d\n", errno);
      return 1;
  } else {
      alarm(CLEAN_FREQ);
  }

  printf("Initializing SDL_net                   ");
  if (SDLNet_Init() == -1) {
      printf("[FAIL]\n");
      fprintf(stderr, "SDLNet_Init(): %s\n", SDLNet_GetError());
      printf("Goodbye.\n");
      exit(EXIT_FAILURE);
  } else printf("[OKAY]\n");

  printf("Opening UDP port %-5d                 ", g_ServerPort);
  if (!(g_ListenSock = SDLNet_UDP_Open(g_ServerPort))) {
      printf("[FAIL]\n");
      fprintf(stderr, "SDLNet_UDP_Open(): %s\n", SDLNet_GetError());
      printf("Goodbye.\n");
      SDLNet_Quit();
      exit(EXIT_FAILURE);
  } else printf("[OKAY]\n");

  printf("Allocating packet buffer               ");
  if (!(recvPacket = SDLNet_AllocPacket(MAX_PACKET))) {
      printf("[FAIL]\n");
      fprintf(stderr, "SDLNet_AllocPacket(): %s\n", SDLNet_GetError());
      printf("Goodbye.\n");
      SDLNet_Quit();
      exit(EXIT_FAILURE);
  } else printf("[OKAY]\n");

  printf("Ready.\n");
  retValue = SDLNet_UDP_Recv(g_ListenSock, recvPacket);
  while ((retValue != -1) && (!g_QuitMainLoop)) {
    if (retValue > 0) {
       if (recvPacket->data[0] == 'M' && recvPacket->data[1] == '+') { // Check for protocol ID
           // Grab version info from header
           majorVersion = recvPacket->data[2];
           minorVersion = recvPacket->data[3];

           // Truncate header so that it's invisible
           recvPacket->data += 4;
           recvPacket->len -= 4;

           if (majorVersion == 1) {
               switch(minorVersion) {
                 case 0:
                     process_packet(recvPacket);
                     break;
               }
           }

           // Fix packet header trunc (this was an annoying segfault)
           recvPacket->data -= 4;
           recvPacket->len += 4;
       }
    }

    retValue = SDLNet_UDP_Recv(g_ListenSock, recvPacket);
  }
 
  // Cleanup
  for (n = 0; n < MAX_GAME_DESC; n++) {
    if (g_GameList[n]) {
      remove_game_desc(n);
    }
  }
  while (g_MD5List != NULL) {
    temp_md5_entry = g_MD5List->next;
    remove_md5_node(g_MD5List);
    g_MD5List = temp_md5_entry;
  }

  SDLNet_FreePacket(recvPacket);
  SDLNet_Quit();

  
  // If server loop kicked due to SDLNet_UDP_Recv error, display it
  if (retValue == -1) fprintf(stderr, "SDLNet_UDP_Recv(): %s\n", SDLNet_GetError());
  printf("Memory leaked: %d\n", g_BytesAllocated);
  printf("Goodbye.\n");
  return EXIT_SUCCESS;
}

/* --------------------------------------------------------------------------------

    process_packet(UDPpacket *packet)

   --------------------------------------------------------------------------------
*/

void process_packet(UDPpacket *packet) {
  int             gameDesc, n;
  md5_byte_t      md5_checksum[16];
  uint16_t        port;
  uint16_t        game;

  // Figure out if entire packet was received, or if buffer was too small
  // If buffer was too small, the packet is malformed

  switch (packet->data[0]) {

    case FIND_MD5:
      if (packet->len == 1) {
        printf("FIND_MD5 for %d.%d.%d.%d\n", GET_IP(packet->address.host));
        send_md5_list(packet->address.host, packet->address.port);
      } else {
        printf("Bad packet length for FIND_MD5 packet from %d.%d.%d.%d.\n", GET_IP(packet->address.host));
      }
      break;

    case FIND_GAMES:
      if (packet->len == 17) {
        memcpy(&md5_checksum, packet->data + 1, 16);
        printf("FIND_GAMES MD5 %X%X for %d.%d.%d.%d\n", md5_checksum[0], md5_checksum[1], GET_IP(packet->address.host));
        send_game_list(packet->address.host, packet->address.port, md5_checksum);
      } else {
        printf("Bad packet length for FIND_GAMES packet from %d.%d.%d.%d.\n", GET_IP(packet->address.host));
      }
      break;

    case OPEN_GAME:
      if (packet->len == 19) {
          // Check to see if the host already has an open game, remove the entry if so
          if ((n = find_game_desc_by_host(packet->address.host)) != -1) {
              printf("Multiple game entries for %d.%d.%d.%d, removing old entry (%d).\n", GET_IP(packet->address.host), n);
              remove_game_desc(n);
          }
          if ((gameDesc = get_free_game_desc()) != -1) { // Find free game descriptor
              memcpy(&md5_checksum, packet->data + 1, 16);
              port = SDLNet_Read16(packet->data + 17);
              g_GameList[gameDesc] = track_malloc(sizeof(GameEntry));
              if (g_GameList[gameDesc]) {
                  g_GameList[gameDesc]->host = packet->address.host; // Network byte order
                  g_GameList[gameDesc]->port = port;
                  g_GameList[gameDesc]->keep_alive = 1;
                  add_game_entry_node(g_GameList[gameDesc], md5_checksum);
                  send_game_descriptor(packet->address.host, packet->address.port, gameDesc);
                  g_GameCount++;
                  printf("OPEN_GAME request for %d.%d.%d.%d:%d %X%X granted (%d).\n", GET_IP(packet->address.host), port, md5_checksum[0], md5_checksum[1], gameDesc);
              } else printf("OPEN_GAME failed, out of memory!\n");

          } else {
              printf("OPEN_GAME request from %d.%d.%d.%d failed (no available game descriptors).\n", GET_IP(packet->address.host));
          }

      } else {
          printf("Bad packet length for OPEN_GAME packet from %d.%d.%d.%d.\n", GET_IP(packet->address.host));
      }
      break;

    case KEEP_ALIVE:
      if (packet->len == 3) {
          game = SDLNet_Read16(packet->data + 1);
          // Make sure the game descriptor indexes a valid pointer in the game list array
          if ((game < MAX_GAME_DESC) && (g_GameList[game] != NULL)) {
            // don't let anybody keep other hosts alive (potential dos)
            if (g_GameList[game]->host == packet->address.host) {
              g_GameList[game]->keep_alive = 1;
//              printf("KEEP_ALIVE request from %d.%d.%d.%d granted.\n", GET_IP(packet->address.host));
            } else {
              // If this happens, someone is screwing around
              printf("Mismatched game descriptor in KEEP_ALIVE packet from %d.%d.%d.%d.\n", GET_IP(packet->address.host));
            }
          } else {
            printf("Bad game descriptor in KEEP_ALIVE packet from %d.%d.%d.%d.\n", GET_IP(packet->address.host));
          }
      } else {
          printf("Bad packet length for KEEP_ALIVE packet from %d.%d.%d.%d.\n", GET_IP(packet->address.host));
      }
      break;

    default:
      printf("Received unrecognized packet from %d.%d.%d.%d, ignoring.\n", GET_IP(packet->address.host));
  }

}
void *track_malloc(int size) {
    void *ptr = malloc(size);
    if (!ptr) printf("malloc(): Failed to allocate memory block.\n");
    else {
      g_BytesAllocated += size;
      g_BlocksAllocated++;
    }
    return ptr;
}

void track_free(void *ptr, int bytes) {
    g_BytesAllocated -= bytes;
    g_BlocksAllocated--;
    free(ptr);
}

// ================================================================================
// == Game Descriptor Functions ===================================================
// ================================================================================

/* --------------------------------------------------------------------------------

    get_free_game_desc()

   --------------------------------------------------------------------------------
*/
int get_free_game_desc() {
  static int currentGameDesc = 0;
  int oldGameDesc;

  if (currentGameDesc == -1) currentGameDesc = 0;
  oldGameDesc = currentGameDesc;
  if (g_GameList[currentGameDesc]) {
      currentGameDesc++;
      while((g_GameList[currentGameDesc]) && (oldGameDesc != currentGameDesc)) {
          currentGameDesc++;
          if (currentGameDesc == MAX_GAME_DESC) currentGameDesc = 0;
      }
      if (g_GameList[currentGameDesc]) currentGameDesc = -1;
  }

  return currentGameDesc;
}

/* --------------------------------------------------------------------------------

    free_timed_out_game_desc()

   --------------------------------------------------------------------------------
*/
void free_timed_out_game_desc() {
  int n;

  for (n = 0; n < MAX_GAME_DESC; n++) {
    if (g_GameList[n]) { // If pointer array element is not null

      if (!g_GameList[n]->keep_alive) { // No keep alive received this round
        printf("Game entry for %d.%d.%d.%d (%d) has timed out.\n", GET_IP(g_GameList[n]->host), n);
        remove_game_desc(n);
      } else {
        g_GameList[n]->keep_alive = 0;
      }
    }
  }
}


/* --------------------------------------------------------------------------------

    find_game_desc_by_host(uint32_t host)

   --------------------------------------------------------------------------------
*/
int find_game_desc_by_host(uint32_t host) {
  int n;
  for (n = 0; n < MAX_GAME_DESC; n++) if ((g_GameList[n]) && (g_GameList[n]->host == host)) break;
  if (n == MAX_GAME_DESC) n = -1;
  return n;
}

/* --------------------------------------------------------------------------------

    remove_game_desc(int n)

   --------------------------------------------------------------------------------
*/
void remove_game_desc(int n) {
  if ((n >= 0) && (n < MAX_GAME_DESC) && (g_GameList[n])) {
    // Remove node from linked list (no need to check for null we have a head and tail node)
    g_GameList[n]->prev->next = g_GameList[n]->next;
    g_GameList[n]->next->prev = g_GameList[n]->prev;

    // Free the node and set the pointer array element to null
    track_free(g_GameList[n], sizeof(GameEntry));
    g_GameList[n] = NULL;
    g_GameCount--;

    // If we've left an empty MD5 list, remove that too
    clean_md5_list();

  } else {
    printf("remove_game_desc(): Invalid game descriptor %d.\n", n);
  }
}

// ================================================================================
// == Signal callbacks ============================================================
// ================================================================================

/* --------------------------------------------------------------------------------

    interupt_pause()

   --------------------------------------------------------------------------------
*/

void interupt_pause(int value) {
    static time_t  last_interupt = 0;

    if (time(NULL) - last_interupt < 5) {
       g_QuitMainLoop = 1;
       printf("Sigint received, cleaning up.\n");
    } else {
       last_interupt = time(NULL);
       printf("\n");
       printf("*********************************************\n");
       printf("**   /~ Mupen64Plus Master Server %s ~\\   **\n", SERVER_VER);
       printf("**           orbitaldecay 2008             **\n");
       printf("**                                         **\n");
       printf("**      Server Uptime:      %4d:%02d:%02d     **\n", ELAPSED(last_interupt - g_StartTime)); 
       printf("**      Server Port:             %5d     **\n", g_ServerPort);
       printf("**      Bytes Allocated:   %11d     **\n", g_BytesAllocated);
       printf("**      Open Games:              %5d     **\n", g_GameCount);
       printf("**                                         **\n");
       printf("**  Sigint received, press CTRL+C to quit  **\n");
       printf("*********************************************\n");
       //print_debug();
    }
}

/* --------------------------------------------------------------------------------

    alarm_func()

   --------------------------------------------------------------------------------
*/
void alarm_func (int sig) {
//  printf("**  Alarm signal received, cleaning game list  **\n");
  free_timed_out_game_desc();
  signal(sig, alarm_func);
  alarm(CLEAN_FREQ);
}

// ================================================================================
// == Linked list functions =======================================================
// ================================================================================

/* --------------------------------------------------------------------------------

    add_game_entry_node(GameEntry *, md5_checksum)

   --------------------------------------------------------------------------------
*/
void add_game_entry_node(GameEntry *newGameEntry, md5_byte_t md5_checksum[16]) {
  MD5Entry *md5_entry;

  if (!(md5_entry = find_md5_node(md5_checksum))) {
      if (!(md5_entry = add_md5_node(md5_checksum))) {
          printf("add_game_entry_node(): Out of memory!\n");
          return;
      }
  }
  newGameEntry->prev = md5_entry->first_game_entry;
  newGameEntry->next = md5_entry->first_game_entry->next;
  md5_entry->first_game_entry->next->prev = newGameEntry;
  md5_entry->first_game_entry->next = newGameEntry;
} 

/* --------------------------------------------------------------------------------

    clean_md5_list()

   --------------------------------------------------------------------------------
*/
void clean_md5_list() {
  MD5Entry *tempNode1, *tempNode2;
  tempNode1 = g_MD5List;

  while (tempNode1) {
      tempNode2 = tempNode1;
      tempNode1 = tempNode1->next;
      if (!tempNode2->first_game_entry->next->next) remove_md5_node(tempNode2);
  }
}
/* --------------------------------------------------------------------------------

    add_md5_node(md5_byte_t md5_checksum[16])

   --------------------------------------------------------------------------------
*/
MD5Entry *add_md5_node(md5_byte_t md5_checksum[16]) {
  MD5Entry *tempNode;

  tempNode = track_malloc(sizeof(MD5Entry));
  if (!tempNode) {
      return NULL;
  }
  memcpy(tempNode->md5_checksum, md5_checksum, 16);

  // Create head gameentry node, check for malloc failure
  if (!(tempNode->first_game_entry = track_malloc(sizeof(GameEntry)))) {
      track_free(tempNode, sizeof(MD5Entry));
      return NULL;
  }

  // Create tail gameentry node, check for malloc failure
  if (!(tempNode->first_game_entry->next = track_malloc(sizeof(GameEntry)))) {
      track_free(tempNode->first_game_entry, sizeof(GameEntry));
      track_free(tempNode, sizeof(MD5Entry));
      return NULL;
  }

  tempNode->first_game_entry->prev = NULL;
  tempNode->first_game_entry->next->prev = tempNode->first_game_entry;
  tempNode->first_game_entry->next->next = NULL;
  tempNode->next = g_MD5List;
  g_MD5List = tempNode;

  return tempNode;
}

/* --------------------------------------------------------------------------------

    find_md5_node(md5_byte_t md5_checksum[16])

   --------------------------------------------------------------------------------
*/
MD5Entry *find_md5_node(md5_byte_t md5_checksum[16]) {
  MD5Entry *tempNode = g_MD5List;

  while (tempNode) {
    if (memcmp(tempNode->md5_checksum, md5_checksum, 16) == 0) break;
    tempNode = tempNode->next;
  }
  return tempNode;
}

/* --------------------------------------------------------------------------------

    remove_md5_node(MD5Entry *deadNode)

   --------------------------------------------------------------------------------
*/
void remove_md5_node(MD5Entry *deadNode) {
  MD5Entry *tempNode = g_MD5List;
  flush_game_entry_list(deadNode);
  if (deadNode == g_MD5List) {
    g_MD5List = deadNode->next;
  } else {
    while((tempNode) && (tempNode->next != deadNode)) {tempNode = tempNode->next;}
    if (tempNode) tempNode->next = deadNode->next;
  }
  track_free(deadNode, sizeof(MD5Entry));
}

/* --------------------------------------------------------------------------------

    flush_game_entry_list(MD5Entry* md5_node)

   --------------------------------------------------------------------------------
*/
void flush_game_entry_list(MD5Entry* md5_node) {
  GameEntry *temp_ge, *temp_ge2;

  temp_ge = md5_node->first_game_entry;
  while (temp_ge) {
      temp_ge2 = temp_ge;
      temp_ge = temp_ge->next;
      track_free(temp_ge2, sizeof(GameEntry));
  }
  md5_node->first_game_entry = NULL;
}

// ================================================================================
// == UDP Packet senders ==========================================================
// ================================================================================

/* --------------------------------------------------------------------------------

    send_game_descriptor(UDPpacket *packet, int gameDesc)

   --------------------------------------------------------------------------------
*/
void send_game_descriptor(uint32_t host, uint16_t port, int gameDesc) {
    UDPpacket *sendPacket;
    if ((sendPacket = SDLNet_AllocPacket(MAX_PACKET)) == NULL) {
        printf("SDLNet_AllocPacket(): %s\n", SDLNet_GetError());
        return;
    }
    sendPacket->address.host = host;
    sendPacket->address.port = port;
    sendPacket->data[0] = GAME_DESC;
    sendPacket->len     = 3;
    SDLNet_Write16(gameDesc, sendPacket->data + 1);
    if (!SDLNet_UDP_Send(g_ListenSock, -1, sendPacket)) {
      printf("SDLNet_UDP_Send(): %s\n", SDLNet_GetError());
    }
    SDLNet_FreePacket(sendPacket);
}

/* --------------------------------------------------------------------------------

    send_game_list(UDPpacket *packet, md5_byte_t md5_checksum[16])
 
   --------------------------------------------------------------------------------
*/
void send_game_list(uint32_t host, uint16_t port, md5_byte_t md5_checksum[16]) {
    MD5Entry  *temp_md5_entry;
    UDPpacket *sendPacket;
    GameEntry *temp_ge;
    int        packet_offset = 2;
    if ((sendPacket = SDLNet_AllocPacket(MAX_PACKET)) == NULL) {
        printf("SDLNet_AllocPacket(): %s\n", SDLNet_GetError());
        return;
    }
    sendPacket->address.host = host;
    sendPacket->address.port = port;
    sendPacket->data[0] = GAME_LIST;
    if ((temp_md5_entry = find_md5_node(md5_checksum))) {
      temp_ge = temp_md5_entry->first_game_entry->next;
      while (temp_ge->next != NULL) {
          if (packet_offset + 6 > MAX_PACKET) {
              sendPacket->len = packet_offset;
              packet_offset = 2;
              sendPacket->data[1] = 1; // Fragmented
              if (!SDLNet_UDP_Send(g_ListenSock, -1, sendPacket)) {
                 printf("SDLNet_UDP_Send(): %s\n", SDLNet_GetError());
              }    
          }
          memcpy(sendPacket->data + packet_offset, &(temp_ge->host), 4);
          SDLNet_Write16(temp_ge->port, sendPacket->data + packet_offset + 4);
          packet_offset += 6;
          temp_ge = temp_ge->next;
      }
    }

    sendPacket->len = packet_offset;
    sendPacket->data[1] = 0;
    if (!SDLNet_UDP_Send(g_ListenSock, -1, sendPacket)) {
        printf("SDLNet_UDP_Send(): %s\n", SDLNet_GetError());
    }
    SDLNet_FreePacket(sendPacket);
}

/* --------------------------------------------------------------------------------

    send_md5_list(UDPpacket *packet, md5_byte_t md5_checksum[16])
 
   --------------------------------------------------------------------------------
*/
void send_md5_list(uint32_t host, uint16_t port) {
}
