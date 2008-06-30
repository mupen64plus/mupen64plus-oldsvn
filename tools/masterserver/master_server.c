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
#include <sys/time.h>
#include <SDL_net.h>

// ================================================================================
// == master_server.h =============================================================
// ================================================================================

#define MAX_PACKET	1470  // In bytes, the largest packet permitted by the protocol
#define SERVER_PORT	2000  // The udp port to run the server on
#define MAX_GAME_DESC   32768 // The highest available game descriptor (max is 65536)
#define CLEAN_FREQ      60    // In seconds, how often free_timed_out_game_desc() is called

#define HEADER_ID       "M64+"

#define FIND_GAMES      00
#define GAME_LIST	01
#define OPEN_GAME       02
#define GAME_DESC	03
#define KEEP_ALIVE      04
#define BAD_VERSION     05

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


void process_packet(UDPpacket *packet);

// Signal callbacks
extern void interupt_trap(int value);
extern void alarm_func(int sig);

// Game Descriptor Functions
void        remove_game_desc(int n);
int         get_free_game_desc();
int         find_game_desc_by_host(uint32_t host);
void        free_timed_out_game_desc();

// UDP Send Data Functions
void        send_game_descriptor(UDPpacket *packet, int gameDesc);
void        send_game_list(UDPpacket *packet, md5_byte_t md5_checksum[16]);

// Linked List Functions
void        add_game_entry_node(GameEntry *newGameEntry, md5_byte_t md5_checksum[16]);
MD5Entry *  add_md5_node(md5_byte_t md5_checksum[16]);
void        remove_md5_node(MD5Entry *deadNode);
MD5Entry *  find_md5_node(md5_byte_t md5_checksum[16]);
void        flush_game_entry_list(MD5Entry* md5_node);

// Global Variables
int           g_QuitMainLoop = 0;
int           g_GameCount = 0;
GameEntry    *g_GameList[MAX_GAME_DESC];
UDPsocket     g_ListenSock;
MD5Entry     *g_MD5List = NULL;

// ================================================================================
// == main() and processPacket() ==================================================
// ================================================================================

/* --------------------------------------------------------------------------------

    main()

   --------------------------------------------------------------------------------
*/

int main(int argc, char **argv) {
  UDPpacket *recvPacket;
  int retValue, n;

  printf("\nMupen64Plus Master Server 0.1 by orbitaldecay\n\n");
  memset(&g_GameList, 0, sizeof(g_GameList));

  // TODO: Process command line arguments here 

  if (signal(SIGINT, SIG_IGN) != SIG_IGN) {
      if (signal(SIGINT, interupt_trap) == SIG_ERR) {
          fprintf(stderr, "signal(): Error %d trapping SIGINT.\n", errno);
      }
  }

  if (signal(SIGALRM, alarm_func) == SIG_ERR) {
      fprintf(stderr, "signal(): Error %d trapping SIGALRM.\n", errno);
  } else {
      alarm(CLEAN_FREQ);
  }
  
  printf("Initializing SDL_net...\t\t");
  if (SDLNet_Init() == -1) {
      printf("[FAIL]\n");
      fprintf(stderr, "SDLNet_Init(): %s\n", SDLNet_GetError());
      printf("Goodbye.\n");
      exit(EXIT_FAILURE);
  } else printf("[Ok]\n");

  printf("Opening UDP port...\t\t");
  if (!(g_ListenSock = SDLNet_UDP_Open(SERVER_PORT))) {
      printf("[FAIL]\n");
      fprintf(stderr, "SDLNet_UDP_Open(): %s\n", SDLNet_GetError());
      printf("Goodbye.\n");
      SDLNet_Quit();
      exit(EXIT_FAILURE);
  } else printf("[Ok]\n");

  printf("Allocating packet buffer...\t");
  if (!(recvPacket = SDLNet_AllocPacket(MAX_PACKET))) {
      printf("[FAIL]\n");
      fprintf(stderr, "SDLNet_AllocPacket(): %s\n", SDLNet_GetError());
      printf("Goodbye.\n");
      SDLNet_Quit();
      exit(EXIT_FAILURE);
  } else printf("[Ok]\n");

  printf("Ready.\n");
  retValue = SDLNet_UDP_Recv(g_ListenSock, recvPacket);
  while ((retValue != -1) && (!g_QuitMainLoop)) {
    if (retValue > 0) {
       // Check header, then process_packet for appropriate version
        process_packet(recvPacket);
    } else {
        // nanosleep?
    }
    retValue = SDLNet_UDP_Recv(g_ListenSock, recvPacket);
  }
 
  // Cleanup
  for (n = 0; n < MAX_GAME_DESC; n++) {
    if (g_GameList[n]) {
      remove_game_desc(n);
    }
  }
  SDLNet_FreePacket(recvPacket);
  SDLNet_Quit();
  
  // If server loop kicked due to SDLNet_UDP_Recv error, display it
  if (retValue == -1) {
    fprintf(stderr, "SDLNet_UDP_Recv(): %s\n", SDLNet_GetError());
    printf("Goodbye.\n");

  // Otherwise we received a sigint, call the default handler
  } else {
    printf("Goodbye.\n");
    signal(SIGINT, SIG_DFL);
    kill(getpid(), SIGINT);
//    while (1) {} // Wait for process to be killed

  /* Useful information regarding proper handling of sigints can be
     found at: http://www.cons.org/cracauer/sigint.html */

  }

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

    case FIND_GAMES:
      if (packet->len == 17) {
        memcpy(&md5_checksum, packet->data + 1, 16);
        send_game_list(packet, md5_checksum);
      } else {
        printf("Bad packet length for FIND_GAMES packet from %d.%d.%d.%d.\n", GET_IP(packet->address.host));
      }
      break;

    case OPEN_GAME:
      if (packet->len == 19) {

          // Check to see if the host already has an open game, remove the entry if so
          if ((n = find_game_desc_by_host(packet->address.host)) != -1) {
              printf("Multiple game entries for %d.%d.%d.%d, removing old entry.\n", GET_IP(packet->address.host));              
              remove_game_desc(n);
          }
          if ((gameDesc = get_free_game_desc()) != -1) { // Find free game descriptor
              memcpy(&md5_checksum, packet->data + 1, 16);
              port = SDLNet_Read16(packet->data + 17);
              g_GameList[gameDesc] = malloc(sizeof(GameEntry));
              g_GameList[gameDesc]->host = packet->address.host; // Network byte order
              g_GameList[gameDesc]->port = port;
              g_GameList[gameDesc]->keep_alive = 1;
              add_game_entry_node(g_GameList[gameDesc], md5_checksum);
              send_game_descriptor(packet, gameDesc);
              g_GameCount++;
              printf("OPEN_GAME request for %d.%d.%d.%d:%d granted (%d).\n", GET_IP(packet->address.host), port, gameDesc);
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
              printf("KEEP_ALIVE request from %d.%d.%d.%d granted.\n", GET_IP(packet->address.host));
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
  for (n = 0; n < MAX_GAME_DESC; n++)
      if ((g_GameList[n]) && (g_GameList[n]->host == host)) break;
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
    free(g_GameList[n]);
    g_GameList[n] = NULL;
    g_GameCount--;
  } else {
    printf("remove_game_desc(): Invalid game descriptor %d.\n", n);
  }
}

// ================================================================================
// == Signal callbacks ============================================================
// ================================================================================

/* --------------------------------------------------------------------------------

    interupt_trap()

   --------------------------------------------------------------------------------
*/
void interupt_trap(int value) {
  printf("!!Interupt signal received, cleaning up!!\n");
  g_QuitMainLoop = 1;
}

/* --------------------------------------------------------------------------------

    alarm_func()

   --------------------------------------------------------------------------------
*/
void alarm_func (int sig) {
//  printf("!!Alarm signal received, cleaning game list!!\n");
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
      md5_entry = add_md5_node(md5_checksum); // This can be customized (so only certain ROMS are available)
  }
  newGameEntry->prev = md5_entry->first_game_entry;
  newGameEntry->next = md5_entry->first_game_entry->next;
  md5_entry->first_game_entry->next = newGameEntry;
} 

/* --------------------------------------------------------------------------------

    add_md5_node(md5_byte_t md5_checksum[16])

   --------------------------------------------------------------------------------
*/
MD5Entry *add_md5_node(md5_byte_t md5_checksum[16]) {
  MD5Entry *tempNode;

  tempNode = malloc(sizeof(MD5Entry));
  memcpy(&(tempNode->md5_checksum), &md5_checksum, sizeof(md5_checksum));
  tempNode->first_game_entry = malloc(sizeof(GameEntry));

  tempNode->first_game_entry->prev = NULL;
  tempNode->first_game_entry->next = malloc(sizeof(GameEntry));
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
    if (memcmp(&(tempNode->md5_checksum), &md5_checksum, sizeof(md5_checksum)) == 0)
        break;
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
  free(deadNode);
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
      free(temp_ge2);
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
void send_game_descriptor(UDPpacket *packet, int gameDesc) {
    packet->data[0] = GAME_DESC;
    packet->len     = 3;
    SDLNet_Write16(gameDesc, packet->data + 1);
    if (!SDLNet_UDP_Send(g_ListenSock, -1, packet)) {
      printf("SDLNet_UDP_Send(): %s\n", SDLNet_GetError());
    }
}

/* --------------------------------------------------------------------------------

    send_game_list(UDPpacket *packet, md5_byte_t md5_checksum[16])

   --------------------------------------------------------------------------------
*/
void send_game_list(UDPpacket *packet, md5_byte_t md5_checksum[16]) {
    MD5Entry  *temp_md5_entry;
    GameEntry *temp_ge;
    int        packet_offset = 2;

    packet->data[0] = GAME_LIST;
    if ((temp_md5_entry = find_md5_node(md5_checksum))) {
      temp_ge = temp_md5_entry->first_game_entry->next;
      while (temp_ge->next != NULL) {
          if (packet_offset + 6 > MAX_PACKET) {
              packet->len = packet_offset;
              packet->data[1] = 1; // Fragmented
              packet_offset = 2;
              if (!SDLNet_UDP_Send(g_ListenSock, -1, packet)) {
                 printf("SDLNet_UDP_Send(): %s\n", SDLNet_GetError());
              }    
          }
          memcpy(packet->data + packet_offset, &(temp_ge->host), 4);
          SDLNet_Write16(temp_ge->port, packet->data + packet_offset + 4);
          packet_offset += 6;
          temp_ge = temp_ge->next;
      }
    }
    packet->len = packet_offset;
    packet->data[1] = 0;
    packet_offset = 0;
    if (!SDLNet_UDP_Send(g_ListenSock, -1, packet)) {
        printf("SDLNet_UDP_Send(): %s\n", SDLNet_GetError());
    }    
}
