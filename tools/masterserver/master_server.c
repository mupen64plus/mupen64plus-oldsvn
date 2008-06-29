/* --------------------------------------------------------------------------------

    master_server.c
    by orbitaldecay

    This is a work in progress, there is some functionality that is still
    missing.  It should be complete soon.

    A discussion of the master server network protocol can be found at
    http://groups.google.com/group/mupen64plus  An abstract is available under the 
    file section entitled master_server_protocol_rev_02.

   --------------------------------------------------------------------------------
*/

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

#define MAX_PACKET	19    // In bytes, the largest packet permitted by the protocol
#define SERVER_PORT	2000  // The udp port to run the server on
#define MAX_GAME_DESC   32768 // The highest available game descriptor (max is 65536)
#define CLEAN_FREQ      60    // In seconds, how often clean_game_list() is called

#define FIND_GAMES      00
#define GAME_LIST	01
#define OPEN_GAME       02
#define GAME_DESC	03
#define KEEP_ALIVE      04

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

typedef struct MD5EntryNode_t {
  md5_byte_t             md5_checksum[16];
  struct GameEntry_t    *first_game_entry;
  struct MD5EntryNode_t *next;
} MD5EntryNode;


extern void interupt_trap(int value);
extern void clean_game_list_alarm (int sig);

void process_packet(UDPpacket *packet);
void remove_game_entry(int n);
int get_free_game_desc();
int find_host_in_game_list(uint32_t host);
void clean_game_list();
void send_game_descriptor(UDPpacket *packet, int gameDesc);
void add_node_to_linked_list(GameEntry *newGameEntry, md5_byte_t md5_checksum[16]);

int        g_QuitMainLoop = 0;
int        g_GameCount = 0;
GameEntry *g_GameList[MAX_GAME_DESC];
UDPsocket  g_ListenSock;

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

  if (signal(SIGALRM, clean_game_list_alarm) == SIG_ERR) {
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
        process_packet(recvPacket);
    } else {

        // nanosleep or clean server list

    }
    retValue = SDLNet_UDP_Recv(g_ListenSock, recvPacket);
  }
 
  // Cleanup
  for (n = 0; n < MAX_GAME_DESC; n++) {
    if (g_GameList[n]) {
      remove_game_entry(n);
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
      } else {
        printf("Bad packet length for FIND_GAMES packet from %d.%d.%d.%d.\n", GET_IP(packet->address.host));
      }
      break;

    case OPEN_GAME:
      if (packet->len == 19) {

          // Check to see if the host already has an open game, remove the entry if so
          if ((n = find_host_in_game_list(packet->address.host)) != -1) {
              printf("Multiple game entries for %d.%d.%d.%d, removing old entry.\n", GET_IP(packet->address.host));              
              remove_game_entry(n);
          }
          if ((gameDesc = get_free_game_desc()) != -1) { // Find free game descriptor
              memcpy(&md5_checksum, packet->data + 1, 16);
              port = SDLNet_Read16(packet->data + 17);
              g_GameList[gameDesc] = malloc(sizeof(GameEntry));
              g_GameList[gameDesc]->host = packet->address.host; // Network byte order
              g_GameList[gameDesc]->port = port;
              g_GameList[gameDesc]->keep_alive = 1;
              add_node_to_linked_list(g_GameList[gameDesc], md5_checksum);
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
// == Functions which help with the g_GameList array ==============================
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

    clean_game_list()

   --------------------------------------------------------------------------------
*/

void clean_game_list() {
  int n;

  for (n = 0; n < MAX_GAME_DESC; n++) {
    if (g_GameList[n]) { // If pointer array element is not null

      if (!g_GameList[n]->keep_alive) { // No keep alive received this round
        printf("Game entry for %d.%d.%d.%d (%d) has timed out.\n", GET_IP(g_GameList[n]->host), n);
        remove_game_entry(n);
      } else {
        g_GameList[n]->keep_alive = 0;
      }
    }
  }
}


/* --------------------------------------------------------------------------------

    find_host_in_game_list(uint32_t host)

   --------------------------------------------------------------------------------
*/

int find_host_in_game_list(uint32_t host) {
  int n;
  for (n = 0; n < MAX_GAME_DESC; n++)
      if ((g_GameList[n]) && (g_GameList[n]->host == host)) break;
  if (n == MAX_GAME_DESC) n = -1;
  return n;
}

/* --------------------------------------------------------------------------------

    remove_game_entry(int n)

   --------------------------------------------------------------------------------
*/

void remove_game_entry(int n) {
  if ((n >= 0) && (n < MAX_GAME_DESC) && (g_GameList[n])) {

    // Remove node from linked list (no need to check for null we have a head and tail node)
//    g_GameList[n]->prev->next = g_GameList[n]->next;
//    g_GameList[n]->next->prev = g_GameList[n]->prev;

    // Free the node and set the pointer array element to null
    free(g_GameList[n]);
    g_GameList[n] = NULL;
    g_GameCount--;
  } else {
    printf("remove_game_entry(): Invalid game descriptor %d.\n", n);
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

    clean_game_list_alarm()

   --------------------------------------------------------------------------------
*/

void clean_game_list_alarm (int sig) {
//  printf("!!Alarm signal received, cleaning game list!!\n");
  clean_game_list();
  signal(sig, clean_game_list_alarm);
  alarm(CLEAN_FREQ);
}

// ================================================================================
// == Linked list functions =======================================================
// ================================================================================

/* --------------------------------------------------------------------------------

    add_node_to_linked_list(GameEntry *, md5_checksum)

   --------------------------------------------------------------------------------
*/
void add_node_to_linked_list(GameEntry *newGameEntry, md5_byte_t md5_checksum[16]) {
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

