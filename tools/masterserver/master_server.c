#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <sys/time.h>
#include <SDL_net.h>

#define MAX_PACKET	19
#define SERVER_PORT	2000
#define MAX_GAME_DESC   32768

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
void process_packet(UDPpacket *packet);
int find_next_game_desc(uint16_t currentGameDesc);
int find_host_in_game_list(uint32_t host);

int        g_QuitMainLoop = 0;
GameEntry *g_GameList[MAX_GAME_DESC];

/* =============================================================================

    main()

   =============================================================================
*/

int main(int argc, char **argv) {
  UDPsocket listenSock;
  UDPpacket *recvPacket;
  int retValue, n;

  printf("\nMupen64Plus Master Server 0.1 by orbitaldecay\n\n");
  memset(&g_GameList, 0, sizeof(g_GameList));

  if (signal(SIGINT, SIG_IGN) != SIG_IGN) {
      if (signal(SIGINT, interupt_trap) == SIG_ERR) {
          fprintf(stderr, "signal(): Error %d.  Cannot trap interupt signal.\n", errno);
      }
  }

  // TODO: Process command line arguments here 

  printf("Initializing SDL_net...\t\t");
  if (SDLNet_Init() == -1) {
      printf("[FAIL]\n");
      fprintf(stderr, "SDLNet_Init(): %s\n", SDLNet_GetError());
      printf("Goodbye.\n");
      exit(EXIT_FAILURE);
  } else printf("[Ok]\n");

  printf("Opening UDP port...\t\t");
  if (!(listenSock = SDLNet_UDP_Open(SERVER_PORT))) {
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
  retValue = SDLNet_UDP_Recv(listenSock, recvPacket);
  while ((retValue != -1) && (!g_QuitMainLoop)) {
    if (retValue > 0) {
        process_packet(recvPacket);
    } else {

        // nanosleep or sweep nonactive server entries

    }
    retValue = SDLNet_UDP_Recv(listenSock, recvPacket);
  }

  // If server loop kicks due to SDLNet_UDP_Recv error, display it and quit
  if (retValue == -1) {
    fprintf(stderr, "SDLNet_UDP_Recv(): %s\n", SDLNet_GetError());
    // Don't quit yet, allow cleanup to occur
  }
  
  // Cleanup
  for (n = 0; n < MAX_GAME_DESC; n++) {
    if (g_GameList[n]) {
      g_GameList[n]->prev->next = g_GameList[n]->next;
      g_GameList[n]->next->prev = g_GameList[n]->prev;
      free(g_GameList[n]);
      g_GameList[n] = NULL;
    }
  }
  SDLNet_FreePacket(recvPacket);
  SDLNet_Quit();
  printf("Goodbye.\n");
  return EXIT_SUCCESS;
}

/* =============================================================================

    process_packet()

   =============================================================================
*/

void process_packet(UDPpacket *packet) {
  static int      nextGameDesc = 0;
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
          nextGameDesc = find_next_game_desc(nextGameDesc);
          if (nextGameDesc != -1) {
              if (find_host_in_game_list(packet->address.host) == -1) {
                  memcpy(&md5_checksum, packet->data + 1, 16);
                  port = SDLNet_Read16(packet->data + 17);
                  g_GameList[nextGameDesc] = malloc(sizeof(GameEntry));
                  g_GameList[nextGameDesc]->host = packet->address.host; // Network byte order
                  g_GameList[nextGameDesc]->port = port;
                  g_GameList[nextGameDesc]->keep_alive = 1;
                  // Add to linked list
              } else {
                  printf("Multiple OPEN_GAME requests from %d.%d.%d.%d ignored.\n", GET_IP(packet->address.host));
              }
          } else {
              printf("OPEN_GAME request from %d.%d.%d.%d failed (server is full).\n", GET_IP(packet->address.host));
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

/* =============================================================================

    interupt_trap()

   =============================================================================
*/

void interupt_trap(int value) {
  printf("!!Interupt signal received, shutting down!!\n");
  g_QuitMainLoop = 1;
}

/* =============================================================================

    find_next_game_desc()

   =============================================================================
*/

int find_next_game_desc(uint16_t currentGameDesc) {
  uint16_t oldGameDesc = currentGameDesc;

  currentGameDesc++;
  while((g_GameList[currentGameDesc]) && (oldGameDesc != currentGameDesc)) currentGameDesc++;
  if (oldGameDesc == currentGameDesc) currentGameDesc = -1;
  return currentGameDesc;
}

/* =============================================================================

    clean_game_list()

    This function should be run once every 60 seconds.

   =============================================================================
*/

void clean_game_list() {
  int n;

  for (n = 0; n < MAX_GAME_DESC; n++) {
    if (g_GameList[n]) { // If pointer array element is not null

      if (!g_GameList[n]->keep_alive) { // No keep alive received this round

        // Remove node from linked list (no need to check for null we have a head and tail node)
        g_GameList[n]->prev->next = g_GameList[n]->next;
        g_GameList[n]->next->prev = g_GameList[n]->prev;

        // Free the node and set the pointer array element to null
        free(g_GameList[n]);
        g_GameList[n] = NULL;

      } else {
        g_GameList[n]->keep_alive = 0;
      }
    }
  }
}

/* =============================================================================

    find_host_in_game_list()

   =============================================================================
*/

int find_host_in_game_list(uint32_t host) {
  int n;
  for (n = 0; n < MAX_GAME_DESC; n++)
      if ((g_GameList[n]) && (g_GameList[n]->host == host)) break;
  if (n == MAX_GAME_DESC) n = -1;
  return n;
}

/* =============================================================================

    gettimeofday_msec()

    might be useful in the future

   =============================================================================
*/
/*
static unsigned int gettimeofday_msec(void)
{
    struct timeval tv;
    unsigned int foo;
    
    gettimeofday(&tv, NULL);
    foo = ((tv.tv_sec % 1000000) * 1000) + (tv.tv_usec / 1000);
    return foo;
}
*/

